import hashlib
from pathlib import Path
import unittest
from tools import recovery_source_shapes as shapes


class SourceShapesTests(unittest.TestCase):
    def integer_candidate(self, source, body, sites, **extra):
        assembly = '.fn F, global\n' + body + '\n.endfn F\n'
        constraints = dict(source_sha256=hashlib.sha256(source).hexdigest(),
            target_assembly=assembly, target_assembly_sha256=hashlib.sha256(assembly.encode()).hexdigest(),
            reviewed_owner_mapping=True, reviewed_scalar_aliases={'s16': 'short'}, integer_owner_sites=sites)
        constraints.update(extra)
        return shapes.enumerate_shapes(source, 'F', ['target_integer_width'], constraints)

    def test_target_counter_width_repairs_and_contrasting_call_cast(self):
        body = 'li r31, 0\nb .L_test\n.L_body:\nextsh r3, r31\nbl Use\naddi r31, r31, 1\n.L_test:\ncmpwi r31, 2\nblt .L_body\nblr'
        for name in ('index', 'member'):
            source = f'void F(void) {{ s16 {name}; for ({name} = 0; {name} < 2; {name}++) {{ Use({name}); }} }}'.encode()
            sites = [{'name': name, 'register': 'r31'}]
            wide = self.integer_candidate(source, body, sites)[0]['source']
            self.assertIn(f'int {name};'.encode(), wide)
            narrow_body = body.replace('cmpwi r31, 2', 'extsh r0, r31\ncmpwi r0, 2')
            self.assertEqual(self.integer_candidate(wide, narrow_body, sites)[0]['source'], source)
            self.assertEqual(self.integer_candidate(source, narrow_body, sites), [])

    def test_promoted_capture_preserves_real_assignment(self):
        source = b'void F(void) { s16 player; player = work->player; Use(array[player]); }'
        body = 'lha r28, 2(r3)\nbl Check\nmulli r4, r28, 36\nblr'
        sites = [{'name': 'player', 'register': 'r28', 'reviewed_assignment_type': 'short'}]
        result = self.integer_candidate(source, body, sites)[0]
        self.assertEqual(result['source'], source.replace(b's16 player;', b'int player;'))
        self.assertEqual(len(result['edits']), 1)
        self.assertFalse(result['authority_advanced'])
        self.assertEqual(result['target_evidence']['function'], 'F')
        for replacement in (b'player += 1; Use(array[player]);', b'player++; Use(array[player]);'):
            with self.assertRaises(ValueError):
                self.integer_candidate(source.replace(b'Use(array[player]);', replacement), body, sites)
        with self.assertRaises(ValueError):
            self.integer_candidate(source, body.replace('lha ', 'lhz '), sites)

    def test_width_repair_rejects_unsafe_or_ambiguous_source(self):
        body = 'li r31, 0\n.L_body:\naddi r31, r31, 1\ncmpwi r31, 2\nblt .L_body\nblr'
        source = b'void F(void) { s16 i; for (i = 0; i < 2; i++) { Use(i); } }'
        sites = [{'name': 'i', 'register': 'r31'}]
        for invalid in (source.replace(b'Use(i)', b'Use(&i)'),
                        source.replace(b'Use(i)', b'i = 100'),
                        source.replace(b's16 i;', b'volatile s16 i;'),
                        source.replace(b'i < 2', b'i < 3'),
                        source.replace(b'Use(i);', b'int i = 3; Use(i);')):
            with self.assertRaises(ValueError):
                self.integer_candidate(invalid, body, sites)
        with self.assertRaisesRegex(ValueError, 'hash mismatch'):
            self.integer_candidate(source, body, sites, target_assembly_sha256='0'*64)
        with self.assertRaisesRegex(ValueError, 'mapping required'):
            self.integer_candidate(source, body, sites, reviewed_owner_mapping=False)

    def test_capspecial_pre_win_function_ranks_shared_producer_not_local_register(self):
        # Frozen PRE-win function, not the winning patch as generator input.
        # Full pre-win TU: 4f6607ce1d17d2d9b9fb547cd8a9556952c77745f53c124c6f66444aa7cb25ac.
        source = (Path(__file__).parent / 'fixtures/capspecial_koopacoin_prewin.c').read_bytes()
        self.assertEqual(hashlib.sha256(source).hexdigest(), '8d706e8315dde58f761e3592b40fb10ebe904be2b053bac598cdd38813444837')
        reviews = dict(closed_macro_context=True, reviewed_type_aliases={'BOOL': 'int'},
                       reviewed_constants={'FALSE': 0})
        census = shapes.analyze_shared_initializers(source, 'ev_CapKoopaCoin', **reviews)
        # The target's already-exact li/stw/stw region is a real producer clue,
        # even though the unresolved register cycle occurs somewhere else.
        self.assertEqual(len(census['sites']), 2)
        site = next(s for s in census['sites'] if 'loseCount' in s['lvalues'])
        target = {k: site[k] for k in ('start_byte', 'end_byte', 'sha256')}
        target.update(source_sha256=hashlib.sha256(source).hexdigest(), producer='loseCount',
                      rationale='Saved losing-team count supplies both flag stores; inspect its shared-value birth.',
                      observed={'artifact_sha256': 'a1799b041c6bb18b9ea60410518007c90887510d9e07288cb9db373525c7679b',
                                'location': 'retail 0x801BF780..0x801BF788',
                                'fact': 'li r27,0; stw r27,0x2c(r1); stw r27,0x28(r1)'})
        analysis = shapes.analyze_shared_initializers(source, 'ev_CapKoopaCoin', target_producer=target, **reviews)
        self.assertTrue(analysis['sites'][0]['target_supported'])
        self.assertEqual(analysis['sites'][0]['sha256'], site['sha256'])
        rows = shapes.enumerate_shapes(source, 'ev_CapKoopaCoin', ['shared_initializer_producer'],
            {'source_sha256': hashlib.sha256(source).hexdigest(), 'shared_initializer_analysis': analysis})
        self.assertEqual(len(rows), 1)
        # Independent winner function extract, full winner TU dc959f93..., had
        # 2668/2668 bytes, zero rows, 111/111 relocations and42 preserved siblings.
        # This is a known-case ranking regression, not a newly discovered crack.
        self.assertEqual(hashlib.sha256(rows[0]['source']).hexdigest(),
                         'c9485726f777fb5192aa0a5fac852c2ce1d3629a9f39bb84070d4cbe2c7545f0')

    def initializer_analysis(self, source, producer='count', **kwargs):
        census = shapes.analyze_shared_initializers(source, 'F', **kwargs)
        site = census['sites'][0]
        target = {k: site[k] for k in ('start_byte', 'end_byte', 'sha256')}
        target.update(source_sha256=hashlib.sha256(source).hexdigest(), producer=producer,
                      rationale='Target count supplies both stores; source hypothesis, not identity proof',
                      observed={'artifact_sha256': 'a'*64, 'location': '0x100..0x108',
                                'fact': 'zero materialized in count register before flag stores'})
        return shapes.analyze_shared_initializers(source, 'F', target_producer=target, **kwargs)

    def test_shared_initializer_renamed_and_nonkoopa(self):
        for scalar, array in [('count', 'flags'), ('pending', 'states')]:
            source = f'void F(void){{ int {scalar}; BOOL {array}[2]; {scalar} = {array}[0] = {array}[1] = FALSE; Use({scalar}, {array}[0]); }}'.encode()
            analysis = self.initializer_analysis(source, scalar, closed_macro_context=True,
                                                  reviewed_type_aliases={'BOOL': 'int'}, reviewed_constants={'FALSE': 0})
            self.assertEqual(analysis['sites'][0]['status'], 'SAFE')
            self.assertEqual(analysis['sites'][0]['producer'], array+'[1]')
            rows = shapes.enumerate_shapes(source, 'F', ['shared_initializer_producer'],
                {'source_sha256': hashlib.sha256(source).hexdigest(), 'shared_initializer_analysis': analysis})
            self.assertEqual(len(rows), 1)
            self.assertIn(f'{array}[0] = {array}[1] = {scalar} = 0;'.encode(), rows[0]['source'])
        source = b'void F(void){ int a; int b; int count; count = a = b = 0; }'
        self.assertEqual(self.initializer_analysis(source, closed_macro_context=True)['sites'][0]['status'], 'SAFE')

    def test_shared_initializer_unknown_and_dangerous(self):
        base = b'void F(void){ int count; int flags[2]; count = flags[0] = flags[1] = 0; }'
        variants = [base.replace(b'int count', b'volatile int count'),
                    base.replace(b'int flags', b'const int flags'),
                    base.replace(b'flags[0] =', b'flags[i++] ='),
                    base.replace(b'= 0;', b'= Next();'),
                    base.replace(b'count =', b'Use(&count); count ='),
                    base.replace(b'count =', b'Use(&(count)); count ='),
                    base.replace(b'count =', b'Use(&flags[0]); count ='),
                    base.replace(b'count =', b'Use(flags); count ='),
                    base.replace(b'int count;', b''),
                    base.replace(b'int count;', b'int count; { int count; }'),
                    base.replace(b'int flags', b'short flags'),
                    b'#define count other\n'+base,
                    base.replace(b'int flags', b'BOOL flags')]
        for source in variants:
            with self.subTest(source=source):
                analysis = self.initializer_analysis(source, closed_macro_context=True)
                self.assertEqual(analysis['sites'][0]['status'], 'UNKNOWN')
                with self.assertRaises(ValueError):
                    shapes.enumerate_shapes(source, 'F', ['shared_initializer_producer'],
                        {'source_sha256': hashlib.sha256(source).hexdigest(), 'shared_initializer_analysis': analysis})
        self.assertEqual(shapes.analyze_shared_initializers(base, 'F')['sites'][0]['status'], 'UNKNOWN')
        for prefix in (b'typedef volatile int BOOL;\n', b'typedef short BOOL;\n'):
            source = prefix+base.replace(b'int flags', b'BOOL flags')
            self.assertEqual(shapes.analyze_shared_initializers(source, 'F', closed_macro_context=True,
                reviewed_type_aliases={'BOOL': 'int'})['sites'][0]['status'], 'UNKNOWN')
        source = b'#define FALSE Next()\n'+base.replace(b'= 0;', b'= FALSE;')
        self.assertEqual(shapes.analyze_shared_initializers(source, 'F', closed_macro_context=True,
            reviewed_constants={'FALSE': 0})['sites'][0]['status'], 'UNKNOWN')

    def test_shared_initializer_binding_and_no_target(self):
        source = b'void F(void){ int count; int a[2]; count = a[0] = a[1] = 0; }'
        analysis = shapes.analyze_shared_initializers(source, 'F', closed_macro_context=True)
        constraints = {'source_sha256': hashlib.sha256(source).hexdigest(), 'shared_initializer_analysis': analysis}
        self.assertEqual(shapes.enumerate_shapes(source, 'F', ['shared_initializer_producer'], constraints), [])
        analysis = self.initializer_analysis(source, closed_macro_context=True)
        constraints['shared_initializer_analysis'] = analysis
        changed = source+b'\n'
        constraints['source_sha256'] = hashlib.sha256(changed).hexdigest()
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(changed, 'F', ['shared_initializer_producer'], constraints)
        analysis['target_producer'].pop('observed')
        constraints['source_sha256'] = hashlib.sha256(source).hexdigest()
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'F', ['shared_initializer_producer'], constraints)

    def test_snapshot_existing_preserves_multiline_format(self):
        for newline in (b'\n', b'\r\n'):
            statement = b'Pay(player, -total);'
            source = newline.join([b'void F(void)', b'{', b'\tint amount;', b'\t'+statement, b'}', b''])
            constraints = self.scalar_constraints(source, 'amount', statement, statement)
            constraints.update(direction='snapshot_existing', argument_index=1,
                               incoming_value_dead_reviewed=True, later_use_safe_reviewed=True)
            constraints['statements'] = constraints['statements'][:1]
            result = shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], constraints)[0]['candidate_source']
            self.assertEqual(result, source.replace(statement, b'amount = total;'+newline+b'\tPay(player, -amount);'))
            del constraints['direction']
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], constraints)

    def test_snapshot_existing_scalar(self):
        for local, rhs, callee in [('amount', 'total', 'Pay'), ('score', 'pending', 'Record')]:
            statement = f'{callee}(player, -{rhs});'.encode()
            source = b'void F(void){ int '+local.encode()+b'; '+statement+b' }'
            constraints = self.scalar_constraints(source, local, statement, statement)
            constraints.update(direction='snapshot_existing', argument_index=1,
                               incoming_value_dead_reviewed=True, later_use_safe_reviewed=True)
            constraints['statements'] = constraints['statements'][:1]
            result = shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], constraints)[0]['candidate_source']
            self.assertIn(f'{local} = {rhs}; {callee}(player, -{local});'.encode(), result)
            constraints['incoming_value_dead_reviewed'] = False
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], constraints)

    def scalar_constraints(self, source, local, first, second):
        return dict(source_sha256=hashlib.sha256(source).hexdigest(), local=local, direction='fold',
                    compatible_value_conversions=True, nonvolatile_scalar_reads_reviewed=True,
                    local_no_alias_reviewed=True, statements=[dict(original=t,
                        start_byte=source.index(t), end_byte=source.index(t)+len(t),
                        sha256=hashlib.sha256(t).hexdigest()) for t in (first, second)])

    def test_sequence_scalar_argument_names(self):
        for local, rhs, call, sign in [('amount', 'total', 'Pay', '-'), ('score', 'pending', 'Record', '+')]:
            first = f'{local} = {rhs};'.encode()
            second = f'{call}(player, {sign}{local});'.encode()
            source = b'void F(void){ int '+local.encode()+b'; '+first+b' '+second+b' }'
            c = self.scalar_constraints(source, local, first, second)
            result = shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], c)[0]['candidate_source']
            self.assertIn(f'{call}(player, {sign}({local} = {rhs}));'.encode(), result)
            self.assertEqual(result.count(first), 0)

    def test_sequence_scalar_argument_rejections(self):
        first = b'amount = total;'
        for decl, gap, call in [(b'volatile int amount;', b'', b'Pay(player, -amount);'),
                (b'int *amount;', b'', b'Pay(player, -amount);'),
                (b'int amount;', b'Side();', b'Pay(player, -amount);'),
                (b'int amount;', b'', b'Pay(amount, -amount);'),
                (b'int amount;', b'', b'Pay(total, -amount);'),
                (b'int amount;', b'', b'Pay(Next(), -amount);'),
                (b'int amount;', b'Use(&amount);', b'Pay(player, -amount);')]:
            source = b'void F(void){ '+decl+b' '+first+b' '+gap+b' '+call+b' }'
            c = self.scalar_constraints(source, 'amount', first, call)
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], c)
        source = b'void F(void){ int amount; amount = total; Pay(player, -amount); }'
        c = self.scalar_constraints(source, 'amount', first, b'Pay(player, -amount);')
        c['statements'][0]['sha256'] = '0'*64
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'F', ['sequence_scalar_argument'], c)

    def test_indexed_base_snapshot_exact_declaration(self):
        source = b'void K(void){ int masuId = GwPlayer[playerNo].masuId; Use(masuId); }'
        declaration = b'int masuId = GwPlayer[playerNo].masuId;'
        pos = source.index(declaration)
        evidence = b'extern GW_PLAYER GwPlayer[GW_PLAYER_MAX];'
        constraints = {'source_sha256': hashlib.sha256(source).hexdigest(), 'element_type': 'GW_PLAYER',
                       'snapshot_name': 'players', 'array_name': 'GwPlayer', 'reviewed_element_types': ['GW_PLAYER'],
                       'array_declaration_reviewed': True,
                       'array_declaration_evidence': {'text': evidence, 'sha256': hashlib.sha256(evidence).hexdigest()},
                       'declaration': {'start_byte': pos, 'end_byte': pos+len(declaration), 'sha256': hashlib.sha256(declaration).hexdigest()}}
        row = shapes.enumerate_shapes(source, 'K', ['indexed_base_snapshot'], constraints)[0]
        self.assertEqual(row['candidate_source'], source.replace(declaration,
            b'GW_PLAYER *players = GwPlayer;\nint masuId = players[playerNo].masuId;'))
        import copy
        for key, value in [('element_type', 'GWPLAYER'), ('snapshot_name', 'masuId')]:
            invalid = dict(constraints, **{key: value})
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(source, 'K', ['indexed_base_snapshot'], invalid)
        invalid = copy.deepcopy(constraints)
        invalid['declaration']['sha256'] = '0'*64
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'K', ['indexed_base_snapshot'], invalid)
        modified = source.replace(b'[playerNo]', b'[playerNo++]')
        invalid = copy.deepcopy(constraints)
        statement = declaration.replace(b'[playerNo]', b'[playerNo++]')
        invalid.update(source_sha256=hashlib.sha256(modified).hexdigest())
        invalid['declaration'].update(end_byte=pos+len(statement), sha256=hashlib.sha256(statement).hexdigest())
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(modified, 'K', ['indexed_base_snapshot'], invalid)

    def sequence_constraints(self, source):
        statements = []
        for text in (b'value = Get(p++);', b'work->field = value;'):
            pos = source.index(text)
            statements.append({'original': text, 'sha256': hashlib.sha256(text).hexdigest(),
                               'start_byte': pos, 'end_byte': pos+len(text)})
        return {'source_sha256': hashlib.sha256(source).hexdigest(), 'local': 'value',
                'compatible_value_conversions': True, 'field_stability_reviewed': True,
                'statements': statements}

    def test_sequence_preserves_live_local_and_call(self):
        source = b'void F(void){ int value; value = Get(p++); work->field = value; Use(value); }'
        constraints = self.sequence_constraints(source)
        row = shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], constraints)[0]
        self.assertIn(b'work->field = (value = Get(p++), value);', row['candidate_source'])
        self.assertIn(b'int value;', row['candidate_source'])
        self.assertIn(b'Use(value);', row['candidate_source'])
        rebased = b'/* shifted */\n'+source
        constraints.update(source_sha256=hashlib.sha256(rebased).hexdigest(), rebind_exact_statements=True)
        self.assertEqual(len(shapes.enumerate_shapes(rebased, 'F', ['sequence_result_consumer'], constraints)), 1)

    def chain_constraints(self, source, first=b'team = GwPlayerConf[i].grpNo;',
                          second=b'lbl_1_bss_BC[i].teamNo = team;'):
        statements = []
        for text in (first, second):
            pos = source.index(text)
            statements.append(dict(original=text, start_byte=pos, end_byte=pos+len(text),
                                   sha256=hashlib.sha256(text).hexdigest()))
        return dict(source_sha256=hashlib.sha256(source).hexdigest(), local='team',
            mode='chained_field_capture', compatible_value_conversions=True,
            field_stability_reviewed=True, closed_macro_context=True,
            nonvolatile_scalar_reads_reviewed=True, local_no_alias_reviewed=True,
            field_bases_stable_reviewed=True, index_stability_reviewed=True,
            stable_field_bases=['GwPlayerConf', 'lbl_1_bss_BC'], immutable_indices=['i'],
            reviewed_identical_conversion_types={'team': 's16', 'GwPlayerConf[i].grpNo': 's16',
                                                 'lbl_1_bss_BC[i].teamNo': 's16'},
            observed_target_rationale='Reviewed shared-value producer closes the target GPR cycle; replay, not discovery.',
            statements=statements)

    def test_reviewed_chained_field_capture(self):
        source = (b'void F(void){ int i; s16 team; for(i=0;i<4;i++){ '
                  b'team = GwPlayerConf[i].grpNo; lbl_1_bss_BC[i].teamNo = team; Use(team); }}')
        row = shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], self.chain_constraints(source))[0]
        self.assertIn(b'team = lbl_1_bss_BC[i].teamNo = GwPlayerConf[i].grpNo;', row['candidate_source'])
        self.assertEqual(len(row['edits']), 1)
        self.assertIn(b's16 team;', row['candidate_source'])
        self.assertFalse(row['authority_advanced'])
        for key in ('closed_macro_context', 'nonvolatile_scalar_reads_reviewed', 'local_no_alias_reviewed',
                    'field_bases_stable_reviewed', 'index_stability_reviewed', 'observed_target_rationale',
                    'immutable_indices', 'stable_field_bases', 'reviewed_identical_conversion_types'):
            invalid = self.chain_constraints(source)
            del invalid[key]
            with self.assertRaises(ValueError, msg=key):
                shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], invalid)
        invalid = self.chain_constraints(source)
        invalid['reviewed_identical_conversion_types']['lbl_1_bss_BC[i].teamNo'] = 'int'
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], invalid)
        for old, new in ((b's16 team;', b'int team;'), (b's16 team;', b'volatile s16 team;'),
                         (b's16 team;', b's16 team; { s16 team; }'),
                         (b'int i;', b'volatile int i;'), (b'int i;', b'int i; { int i; }'),
                         (b's16 team;', b's16 team; int GwPlayerConf;'),
                         (b's16 team;', b's16 team; Escape(&i);'),
                         (b's16 team;', b's16 team; Escape(&(team));'),
                         (b'; lbl_1', b'; i++; lbl_1'), (b'; lbl_1', b'; Work(); lbl_1')):
            changed = source.replace(old, new)
            with self.assertRaises(ValueError, msg=str(new)):
                shapes.enumerate_shapes(changed, 'F', ['sequence_result_consumer'], self.chain_constraints(changed))
        for first, second in ((b'team = GwPlayerConf[i++].grpNo;', b'lbl_1_bss_BC[i].teamNo = team;'),
                              (b'team = Get();', b'lbl_1_bss_BC[i].teamNo = team;'),
                              (b'team = GwPlayerConf[i].grpNo;', b'lbl_1_bss_BC[team].teamNo = team;'),
                              (b'team = GwPlayerConf[i].grpNo;', b'Get()[i].teamNo = team;')):
            changed = b'void F(void){ int i; s16 team; '+first+b' '+second+b' }'
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(changed, 'F', ['sequence_result_consumer'], self.chain_constraints(changed, first, second))
        invalid = self.chain_constraints(source)
        invalid['statements'][0]['sha256'] = '0'*64
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], invalid)
        invalid = self.chain_constraints(source)
        invalid['source_sha256'] = '0'*64
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], invalid)

    def test_m635_chained_capture_immutable_source_replay(self):
        import os
        import subprocess
        if os.environ.get('SOURCE_SHAPES_M635_REPLAY') != '1':
            self.skipTest('explicit local immutable m635 source replay not selected')
        root = Path(__file__).resolve().parents[2]
        source = subprocess.check_output(['git', 'show',
            '17f1a17bd0e685ec7f1bfc9e4aa6ab1d485f1f2f:src/REL/m635dll/players.c'], cwd=root)
        row = shapes.enumerate_shapes(source, 'fn_1_2954', ['sequence_result_consumer'], self.chain_constraints(source))[0]
        self.assertIn(b'team = lbl_1_bss_BC[i].teamNo = GwPlayerConf[i].grpNo;', row['candidate_source'])
        current = (root/'src/REL/m635dll/graphics.c').read_bytes()
        self.assertEqual(shapes.fingerprint_function(row['candidate_source'], 'fn_1_2954')['token_sha256'],
                         shapes.fingerprint_function(current, 'fn_1_2954')['token_sha256'])
        print('m635 immutable chained-capture replay:', row['source_sha256'], row['candidate_sha256'])

    def test_sequence_rejects_stale_nonadjacent_or_unknown_ownership(self):
        base = b'void F(void){ int value; value = Get(p++); work->field = value; }'
        for source in (base.replace(b'; work', b'; SideEffect(); work'), base.replace(b'int value;', b'')):
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], self.sequence_constraints(source))
        constraints = self.sequence_constraints(base)
        constraints['statements'][0]['sha256'] = '0'*64
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(base, 'F', ['sequence_result_consumer'], constraints)
        constraints = self.sequence_constraints(base)
        constraints['field_stability_reviewed'] = False
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(base, 'F', ['sequence_result_consumer'], constraints)
        source = base.replace(b'work->field', b'GetWork()->field')
        constraints = self.sequence_constraints(base)
        text = b'GetWork()->field = value;'
        pos = source.index(text)
        constraints.update(source_sha256=hashlib.sha256(source).hexdigest())
        constraints['statements'][1] = {'original': text, 'sha256': hashlib.sha256(text).hexdigest(),
                                         'start_byte': pos, 'end_byte': pos+len(text)}
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(source, 'F', ['sequence_result_consumer'], constraints)

    source = b'void Teresa(void) { for (i=0;i<4;i++) { if (!PlayerCheck(i,p) && StarGet(i)>0) { count++; } } }'

    def constraints(self, source=None):
        source = source or self.source
        start = source.index(b'if (')
        end = source.index(b'}', start) + 1
        return {"source_sha256": hashlib.sha256(source).hexdigest(), "integral_calls": ["PlayerCheck", "StarGet"],
                "guard_sites": [{"start_byte": start, "end_byte": end,
                                 "sha256": hashlib.sha256(source[start:end]).hexdigest()}]}

    def test_guard_deterministic_existing_calls(self):
        result = shapes.enumerate_shapes(self.source, 'Teresa', ['loop_predicate_guards'], self.constraints())
        self.assertEqual(len(result), 1)
        self.assertIn(b'if (PlayerCheck(i,p)) { continue; }', result[0]['source'])
        self.assertEqual(result, shapes.enumerate_shapes(self.source, 'Teresa', ['loop_predicate_guards'], self.constraints()))

    def test_fingerprint_whitespace_and_comments(self):
        a = b'void F(void){int n=0; n++;}'
        b = b'void F ( void ) { /* hi */ int n = 0; n ++ ; }'
        self.assertEqual(shapes.fingerprint_function(a, 'F')['token_sha256'], shapes.fingerprint_function(b, 'F')['token_sha256'])
        for c in [b'void F(void){int n; n=0; n++;}', b'void F(void){int n=(int)0; n++;}']:
            self.assertNotEqual(shapes.fingerprint_function(a, 'F')['token_sha256'], shapes.fingerprint_function(c, 'F')['token_sha256'])

    def test_snapshot_same_live_owners(self):
        source = b'void Koopa(void) { s16 src; s16 dst; p=0; src=GwPlayer[p].coin; dst=src; if(dst<=0){use(dst);} }'
        constraints = {"source_sha256": hashlib.sha256(source).hexdigest(),
                       "snapshot_sites": [{"source_local": "src", "result_local": "dst", "player_local": "p"}]}
        result = shapes.enumerate_shapes(source, 'Koopa', ['snapshot_point_use'], constraints)
        self.assertIn(b's16 src=GwPlayer[p].coin;', result[0]['source'])
        self.assertIn(b's16 dst=src;', result[0]['source'])

    def test_unsafe_guard_refusals(self):
        for source in [self.source.replace(b'count++;', b'break;'),
                       self.source.replace(b' } } }', b' } tail(); } }'),
                       self.source.replace(b'count++;', b'again: count++;'),
                       self.source.replace(b'count++;', b'continue;')]:
            with self.assertRaises(ValueError):
                shapes.enumerate_shapes(source, 'Teresa', ['loop_predicate_guards'], self.constraints(source))
        constraints = self.constraints()
        constraints['integral_calls'] = []
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(self.source, 'Teresa', ['loop_predicate_guards'], constraints)

    def test_stale_and_parser_fail_closed(self):
        constraints = self.constraints()
        constraints['source_sha256'] = '0'*64
        with self.assertRaises(ValueError):
            shapes.enumerate_shapes(self.source, 'Teresa', ['loop_predicate_guards'], constraints)
        for source in [b'void F(void){return __LINE__;}', b'void F(void){\n#if X\nf();\n#endif\n}', b'void F(void){int =;}']:
            with self.assertRaises(ValueError):
                shapes.fingerprint_function(source, 'F')

    def test_reuse_requires_lines_outside_and_macro_proof(self):
        def fp(raw, closed=False):
            return shapes.fingerprint_function(raw, 'F', closed_macro_context=closed)
        def duplicate(a, b):
            return shapes.known_shape_duplicate(a, b, compile_context_sha256='a'*64,
                                                known_compile_context_sha256='a'*64)
        a = b'void F(void){int n=0;}'
        spaced = b'void F(void){ int n = 0; }'
        self.assertTrue(duplicate(fp(a), fp(a)))
        self.assertFalse(duplicate(fp(a), fp(spaced)))
        self.assertTrue(duplicate(fp(a, True), fp(spaced, True)))
        multiline = b'void F(void){/* hello\nworld */int n=0;}'
        self.assertEqual(fp(a)['lexical_sha256'], fp(multiline)['lexical_sha256'])
        self.assertFalse(duplicate(fp(a, True), fp(multiline, True)))
        self.assertFalse(duplicate(fp(a, True), fp(b'int outside;\n'+a, True)))
        for macro in [b'__LINE__', b'__FILE__', b'__COUNTER__']:
            with self.assertRaises(ValueError):
                fp(b'#define HIDDEN '+macro+b'\n'+a, True)

    def test_unrelated_cleanup_goto_allowed(self):
        source = self.source[:-1] + b' goto cleanup; cleanup: return; }'
        result = shapes.enumerate_shapes(source, 'Teresa', ['loop_predicate_guards'], self.constraints(source))
        self.assertIn(b'continue;', result[0]['candidate_source'])


if __name__ == '__main__':
    unittest.main()
