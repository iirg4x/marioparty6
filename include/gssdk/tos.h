/* TOS processing blocks, queue ports and control interfaces. */
#ifndef GSSDK_TOS_H
#define GSSDK_TOS_H

#include "types.h"
#include "gssdk/fastallo.h"

/* Scheduler state shared by blocks and queues. */
struct _TOS;
/* Queue that holds the elements waiting for its readers. */
struct _QUEUE;
/* State common to every processing block. */
struct _BLOCK;

/* Identifies a TOS object to the profile and error-log routines. */
struct _OBJECT {
    /* Scheduler that owns the object. */
    struct _TOS *pTos;
    /* Object identifier in the scheduler's block or queue table. */
    u8 BlockID;
};

/* Input queue, reader position, and element size. */
struct _INPUTQUEUE {
    /* Queue connected to the port. */
    struct _QUEUE *pQ;
    /* Index of this input in its queue's reader positions. */
    u16 ReadPos;
    /* Size in bytes of an element at this port. */
    u16 Size;
};

/* Output queue and its element size. */
struct _OUTPUTQUEUE {
    /* Queue connected to the port. */
    struct _QUEUE *pQ;
    /* Size in bytes of an element at this port. */
    u16 Size;
};

/* Callbacks supplied by the recognition architecture. */
struct _TOSDATA {
    /* Called after changing the active control state. */
    u8 (*cbState)(struct _TOS *, u16);
    /* Initializes the architecture when the queue counts are known. */
    u8 (*cbInitQueues)(struct _TOS *, u8, u8);
    /* Registers a queue and its reader position as an input. */
    u8 (*cbRegInput)(struct _TOS *, struct _QUEUE *, u16);
    /* Registers a queue as an output of the architecture. */
    u8 (*cbRegOutput)(struct _TOS *, struct _QUEUE *);
    /* Receives the stored block ID and error code. */
    void (*cbError)(struct _TOS *, u32);
    /* Receives the stored block ID and warning code. */
    void (*cbWarning)(struct _TOS *, u32);
};

/* Floating-point entry in a block profile. */
struct _KEYVAL_FLT {
    /* Key looked up by the block. */
    u16 KeyID;
    /* Value returned for this key. */
    f32 Val;
};

/* Integer entry in a block profile. */
struct _KEYVAL_U32 {
    /* Key looked up by the block. */
    u16 KeyID;
    /* Value returned for this key. */
    u32 Val;
};

/* Profile lookup tables for floating-point and integer settings. */
struct _KEYVALS {
    /* Floating-point settings for the object. */
    const struct _KEYVAL_FLT *pKeyValFLT;
    /* Integer settings for the object. */
    const struct _KEYVAL_U32 *pKeyValU32;
};

/* Construction descriptor for a processing block. */
struct _PROCDESCRIP {
    /* Identifier of the block to create. */
    u8 BlockID;
    /* Creates the block during scheduler initialization when set. */
    u8 Enabled;
    /* Allocates and initializes an instance of the block. */
    struct _BLOCK *(*Constructor)(struct _TOS *This, u8 BlockID);
};

/* Identifies the block that writes to a queue. */
struct _QDESCRIP {
    /* Identifier of the queue. */
    u8 BlockID;
    /* Creates the queue during scheduler initialization when set. */
    u8 Enabled;
    /* Identifier of the block that writes the queue. */
    u8 SourceID;
};

/* Identifies a block that reads from a queue. */
struct _QTARGETDESCRIP {
    /* Identifier of the queue. */
    u8 BlockID;
    /* Identifier of the block that reads the queue. */
    u8 TargetID;
};

/* A command forwarded to another block or queue. */
struct _CTRL {
    /* Identifier of the receiving block or queue. */
    u8 BlockID;
    /* Control ID passed to the recipient. */
    u8 ControlID;
};

/* Handles a scheduler control command and returns its status. */
typedef u32 (*ControlCallback)(struct _TOS *pTos, u8 BlockID, u32 ControlID,
                               void *CParam1, u32 CParam2);

/* Callback and state transition for a routed control command. */
struct _CNTRL0 {
    /* Incoming control ID that selects the route. */
    u8 ControlID;
    /* Runs the scheduler after each forwarded control when set. */
    u8 SyncBlock;
    /* State to select before dispatch, or a relative transition code. */
    u16 NextState;
    /* Optional callback invoked after dispatching every target. */
    ControlCallback CallBackFunc;
    /* Targets followed by a record with ControlID set to 0. */
    struct _CTRL OutputControl[1];
};

/* Control commands that a block can issue. */
struct _INBLOCK {
    /* Identifier of the block that issues the commands. */
    u8 BlockID;
    /* Null-terminated array of controls accepted from the block. */
    const struct _CNTRL0 *const *ppInControl;
};

/* Processing graph and scheduler state for one engine. */
struct _TOS {
    /* Number of slots in the block pointer array. */
    u16 NrProces;
    /* Block ID represented by slot zero. */
    u16 BlockOffset;
    /* Blocks indexed relative to BlockOffset. */
    struct _BLOCK **AllProcesses;
    /* Number of slots in the queue pointer array. */
    u16 NrQueues;
    /* Queue ID represented by slot zero. */
    u16 QueueOffset;
    /* Queues indexed relative to QueueOffset. */
    struct _QUEUE **AllQueues;
    /* Callbacks supplied by the recognition architecture. */
    struct _TOSDATA TosData;
    /* Setting tables for the processing objects. */
    const struct _KEYVALS *IniArray;
    /* Control routes indexed first by bank and then by ActiveState. */
    const struct _INBLOCK *const *const *ControlTable;
    /* Active bank of control routes. */
    u16 ActiveMode;
    /* State within the active bank of control routes. */
    u16 ActiveState;
    /* Last TOS error recorded for the graph. */
    u32 ErrorLog;
    /* Last TOS warning recorded for the graph. */
    u32 WarningLog;
    /* Count of enqueues during the current scheduler pass. */
    s32 NbrOfEnqueues;
    /* Descriptors used to create the processing blocks. */
    const struct _PROCDESCRIP *pInitialBlocks;
    /* Descriptors used to create the queues and connect their writers. */
    const struct _QDESCRIP *pInitialQueues;
    /* Input connections used to register readers. */
    const struct _QTARGETDESCRIP *pQueueReaders;
    /* Array of input payloads for the next processing call. */
    void **pInputsToBlocks;
    /* Heap used for the blocks, queues and their data. */
    void *hHeap;
    /* Chunk allocator for small queue elements. */
    struct _FASTALLO_INST FastAllocHeap;
};

/* Shared block state before the block-specific fields. */
struct _BLOCK {
    /* Block identity used by the profile and Control functions. */
    struct _OBJECT Object;
    /* Number of input ports. */
    u8 NrInputs;
    /* Number of output ports. */
    u8 NrOutputs;
    /* Array of input port records. */
    struct _INPUTQUEUE *InputQueues;
    /* Array of output port records. */
    struct _OUTPUTQUEUE *OutputQueues;
    /* Initializes ports and block state. A nonzero result reports failure. */
    u8 (*Init)(struct _BLOCK *);
    /* Processes an array of queue input pointers and its entry count. */
    void (*Process)(struct _BLOCK *, void *, int);
    /* Handles a routed command and its two payload words. */
    u8 (*Control)(struct _BLOCK *, u8, u32, u32);
    /* Whether the block participates in processing. */
    u8 Enabled;
    /* Processing function retained when the block is disabled. */
    void (*SavedProcess)(struct _BLOCK *, void *, int);
};

/* Reads an integer setting or returns the supplied default. */
u32 _tosGetProfileU32(
    struct _OBJECT *pObject, u16 KeyID, u32 RetVal);

/* Reads a floating-point setting or returns the supplied default. */
f32 _tosGetProfileFloat(
    struct _OBJECT *pObject, u16 KeyID, f32 RetVal);

/* Records a TOS error from the object identified by owner. */
u8 _tosErrorLog(struct _OBJECT *pObject, u16 ErrorCode);

/* Records a warning and notifies the registered callback. */
u8 _tosWarningLog(struct _OBJECT *pObject, u16 WarningCode);

/* Looks up a queue by its scheduler ID. */
struct _QUEUE *tosGetQueuePtr(struct _TOS *This, u8 BlockID);

/* Dispatches the target command to a block or queue. */
u8 tosCallControlFunc(
    struct _TOS *This, const struct _CTRL *pControl,
    u32 CParam, u32 DParam);

/* Initializes scheduler state and builds the processing graph. */
u8 tosInit(
    struct _TOS *pTos, void *hHeap,
    const struct _TOSDATA *pTosData,
    const struct _KEYVALS *IniArray,
    const struct _PROCDESCRIP *InitialProcs,
    const struct _QDESCRIP *InitialQueues,
    const struct _QTARGETDESCRIP *QueueReaders,
    const struct _INBLOCK *const *const *ControlTable,
    u16 InitialMode, u16 InitialState);

/* Releases the graph and the scheduler's allocations. */
u8 tosDestruct(struct _TOS *pTos);

/* Processes ready blocks in schedule order. */
u8 tosRun(struct _TOS *pTos, const u8 *pBlockIDs);

/* Routes a control command from the given object. */
u8 _tosControl(
    struct _OBJECT *pObject, u32 ControlID, u32 CParam1, u32 CParam2);

/* Releases a processing block's storage. */
void tosBaseBlockDestruct(struct _BLOCK *pBaseBlock);

/* Allocates a processing block and its port records. */
struct _BLOCK *tosBaseBlockConstruct(
    struct _TOS *pTos, u8 BlockID, u8 NbrInputs,
    u8 NbrOutputs, void (*Proces)(struct _BLOCK *, void *, int),
    u8 (*Init)(struct _BLOCK *), u8 (*Control)(struct _BLOCK *, u8, u32, u32), u32 BlockSize);

/* Disables the readers connected to the block's inputs. */
u8 tosBaseBlockDisableInputQueues(struct _BLOCK *pBaseBlock);

/* Starts the block's input readers at the requested history depth. */
u8 tosBaseBlockStartInputQueues(
    struct _BLOCK *pBaseBlock, u32 JumpBack);

/* Handles the control commands common to processing blocks. */
u8 tosBaseBlockControl(
    struct _BLOCK *pBaseBlock, u8 ControlId, u32 CParam,
    u32 DParam);

/* Control routes indexed by recognition mode, then scheduler state. */
extern const struct _INBLOCK *const *const ControlTable[];

/* Speech recognition profile keyed by block and queue ID. */
extern const struct _KEYVALS IniFile[87];

/* Block processes installed when TOS opens the architecture. */
extern const struct _PROCDESCRIP InitialProcs[];
/* Output queues created by the recognizer architecture. */
extern const struct _QDESCRIP QueueWriters[];
/* Input queue readers of the recognizer architecture. */
extern const struct _QTARGETDESCRIP QueueReaders[];

#endif
