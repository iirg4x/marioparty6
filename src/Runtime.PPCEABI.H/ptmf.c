/* Only the routines the game links are present. */

/* A member-function descriptor holds an object adjustment and an encoded target. */

typedef struct PTMF {
	long this_delta;
	long v_offset;  
	union {
		void* f_addr;  
		long ve_offset;
	} f_data;
} PTMF;

/* The null descriptor has no adjustment or target. */
const PTMF __ptmf_null = { 0, 0, 0 };

long __ptmf_test(PTMF *ptmf);
long __ptmf_cmpr(PTMF *ptmf1, PTMF *ptmf2);
void __ptmf_scall(...);

/* The C short-circuit test differs; assembly retains the oracle condition-register returns. */
asm long __ptmf_test(PTMF* ptmf)
{
	nofralloc
		lwz       r5, PTMF.this_delta(r3)
		lwz       r6, PTMF.v_offset(r3)
		lwz       r7, PTMF.f_data(r3)
		li        r3, 0x1
		cmpwi     r5, 0
		cmpwi     cr6, r6, 0
		cmpwi     cr7, r7, 0
		bnelr-
		bnelr-    cr6
		bnelr-    cr7
		li        r3, 0
		blr
}

/* The C short-circuit comparison differs; assembly retains the oracle condition-register
 * returns.
 */
asm long __ptmf_cmpr(PTMF *ptmf1, PTMF *ptmf2)
{
    nofralloc
        lwz r5, PTMF.this_delta(r3)
        lwz r6, PTMF.this_delta(r4)
        lwz r7, PTMF.v_offset(r3)
        lwz r8, PTMF.v_offset(r4)
        lwz r9, PTMF.f_data(r3)
        lwz r10, PTMF.f_data(r4)
        li r3, 1
        cmpw r5, r6
        cmpw cr6, r7, r8
        cmpw cr7, r9, r10
        bnelr-
        bnelr- cr6
        bnelr- cr7
        li r3, 0
        blr
}

/* The descriptor in r12 and preserved call arguments require an ABI assembly entry. */
asm void __ptmf_call(...)
{
    nofralloc
    lwz r0, PTMF.this_delta(r12)
    lwz r11, PTMF.v_offset(r12)
    lwz r12, PTMF.f_data(r12)
    cmpwi r11, 0
    add r3, r3, r0
    blt- dispatch_target
    lwzx r12, r3, r12
    add r12, r12, r11
    lwz r0, 0x4(r12)
    lwz r12, 0x0(r12)
    add r3, r3, r0
dispatch_target:
    mtctr r12
    bctr
}

/* The descriptor in r12 and preserved call arguments require an ABI assembly entry. */
asm void __ptmf_call4(...)
{
    nofralloc
    lwz r0, PTMF.this_delta(r12)
    lwz r11, PTMF.v_offset(r12)
    lwz r12, PTMF.f_data(r12)
    cmpwi r11, 0
    add r4, r4, r0
    blt- dispatch_target
    lwzx r12, r4, r12
    add r12, r12, r11
    lwz r0, 0x4(r12)
    lwz r12, 0x0(r12)
    add r4, r4, r0
dispatch_target:
    mtctr r12
    bctr
}

/* The descriptor in r12 and preserved call arguments require an ABI assembly entry. */
asm void __ptmf_scall(...)
{
	nofralloc
		lwz     r0, PTMF.this_delta(r12)
		lwz     r11, PTMF.v_offset(r12)
		lwz     r12, PTMF.f_data(r12)
		add     r3, r3, r0
		cmpwi   r11, 0
		blt-    cr0, loc_0x20
		lwzx    r12, r3, r12
		lwzx    r12, r12, r11
	loc_0x20:
		mtctr   r12
		bctr
}

/* The descriptor in r12 and preserved call arguments require an ABI assembly entry. */
asm void __ptmf_scall4(...)
{
	nofralloc
		lwz     r0, PTMF.this_delta(r12)
		lwz     r11, PTMF.v_offset(r12)
		lwz     r12, PTMF.f_data(r12)
		add     r4, r4, r0
		cmpwi   r11, 0
		blt-    cr0, loc_0x20
		lwzx    r12, r4, r12
		lwzx    r12, r12, r11
	loc_0x20:
		mtctr   r12
		bctr
}

/* Adjust the object displacement while preserving the encoded call target. */
PTMF *__ptmf_cast(long adjustment, const PTMF *source, PTMF *destination)
{
    destination->this_delta = source->this_delta + adjustment;
    destination->v_offset = source->v_offset;
    destination->f_data = source->f_data;
    return destination;
}
