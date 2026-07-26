.fn ExitWatch, local
stwu r1, -0x20(r1)
mflr r0
stw r0, 0x24(r1)
stw r31, 0x1c(r1)
stw r30, 0x18(r1)
stw r29, 0x14(r1)
b .L_8014AA08
.L_8014A9EC:
bl mbPauseStartCheck
mr r29, r3
cmpwi r29, 0x0
blt .L_8014AA04
mr r3, r29
bl mbPauseCreate
.L_8014AA04:
bl HuPrcVSleep
.L_8014AA08:
lwz r0, exitReq@sda21(r0)
cmpwi r0, 0x0
bne .L_8014AA20
lha r0, omSysExitReq@sda21(r0)
cmpwi r0, 0x0
beq .L_8014A9EC
.L_8014AA20:
li r0, 0x1
stw r0, exitFlag@sda21(r0)
b .L_8014AA30
.L_8014AA2C:
bl HuPrcVSleep
.L_8014AA30:
bl WipeCheck
clrlwi r0, r3, 24
cmplwi r0, 0x0
bne .L_8014AA2C
b .L_8014AA48
.L_8014AA44:
bl HuPrcVSleep
.L_8014AA48:
bl HuARDMACheck
cmpwi r3, 0x0
bne .L_8014AA44
lwz r3, mbObjMan@sda21(r0)
li r4, 0x2
bl HuPrcResetStat
lwz r3, mbObjMan@sda21(r0)
lwz r31, 0x12c(r3)
li r30, 0x0
b .L_8014AAA0
.L_8014AA70:
lwz r3, 0xc(r31)
mulli r0, r30, 0x60
lhzx r0, r3, r0
andi. r0, r0, 0x21
cmpwi r0, 0x0
bne .L_8014AA9C
lwz r3, 0xc(r31)
mulli r0, r30, 0x60
add r3, r3, r0
li r4, 0x10
bl omResetStatBit
.L_8014AA9C:
addi r30, r30, 0x1
.L_8014AAA0:
lha r0, 0x0(r31)
cmpw r30, r0
blt .L_8014AA70
bl mbPauseEnableReset
bl HuPrcVSleep
li r3, 0x0
bl omSysPauseCtrl
li r3, 0x0
bl HuPrcAllPause
li r3, 0x0
bl Hu3DPauseSet
li r3, 0x0
bl HuSprPauseSet
lwz r3, mbObjMan@sda21(r0)
bl HuPrcKill
bl HuPrcEnd
lwz r31, 0x1c(r1)
lwz r30, 0x18(r1)
lwz r29, 0x14(r1)
lwz r0, 0x24(r1)
mtlr r0
addi r1, r1, 0x20
blr
.endfn ExitWatch
