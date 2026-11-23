/* Speech recognition context records and their accessors. */
#ifndef GSSDK_CONTEXT_H
#define GSSDK_CONTEXT_H

/* Version-two context data and the functions that read it. */
struct _ASRSPI_CONTEXT {
    /* Address of the packed context data. */
    void *pCtx;
    /* Checks the context's format identifier. */
    int (*contextCheckDataType)(struct _ASRSPI_CONTEXT *);
    /* Returns the context data size. */
    unsigned long (*contextGetSize)(struct _ASRSPI_CONTEXT *);
    /* Returns the language version string required by the context. */
    char *(*contextGetLanguageVersionInfo)(struct _ASRSPI_CONTEXT *);
    /* Returns the context's terminal silence setting. */
    unsigned long (*contextGetTS)(struct _ASRSPI_CONTEXT *);
    /* Returns the configured recognition accuracy. */
    unsigned long (*contextGetAccuracy)(struct _ASRSPI_CONTEXT *);
    /* Returns the configured rejection setting. */
    unsigned long (*contextGetRejection)(struct _ASRSPI_CONTEXT *);
    /* Returns the configured garbage-word setting. */
    unsigned long (*contextGetGarbage)(struct _ASRSPI_CONTEXT *);
    /* Returns the stored syntax-item count. */
    unsigned long (*contextGetNbrItem)(struct _ASRSPI_CONTEXT *);
    /* Returns the stored context word count. */
    unsigned long (*contextGetNbrWord)(struct _ASRSPI_CONTEXT *);
    /* Returns the stored pronunciation count. */
    unsigned long (*contextGetNbrPron)(struct _ASRSPI_CONTEXT *);
    /* Returns the stored follower count. */
    unsigned long (*contextGetNbrFollower)(struct _ASRSPI_CONTEXT *);
    /* Returns the stored count of words that can start a match. */
    unsigned long (*contextGetNbrStartWord)(struct _ASRSPI_CONTEXT *);
    /* Returns the stored count of words that can end a match. */
    unsigned long (*contextGetNbrStopWord)(struct _ASRSPI_CONTEXT *);
    /* Returns the ID used for garbage-word recognition. */
    unsigned long (*contextGetGarbageWordID)(struct _ASRSPI_CONTEXT *);
    /* Reports whether the context uses the spelling recognizer. */
    unsigned char (*contextIsSpellingContext)(struct _ASRSPI_CONTEXT *);
    /* Returns the pronunciation lexicon. */
    unsigned short *(*contextGetpLex)(struct _ASRSPI_CONTEXT *);
    /* Returns the word offsets into the context data. */
    unsigned long *(*contextGetpBeginOfWords)(struct _ASRSPI_CONTEXT *);
    /* Returns the word properties. */
    unsigned short *(*contextGetpWordProp)(struct _ASRSPI_CONTEXT *);
    /* Returns the syntax data. */
    unsigned long *(*contextGetpSyntax)(struct _ASRSPI_CONTEXT *);
    /* Reports whether the context uses isolated-word recognition. */
    unsigned char (*contextIsIsoWordContext)(struct _ASRSPI_CONTEXT *);
};

/* Installs the accessors for version-two context data. */
void FillContextV2VirtualTable(struct _ASRSPI_CONTEXT *pCtx);

#endif
