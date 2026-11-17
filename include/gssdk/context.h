#ifndef GSSDK_CONTEXT_H
#define GSSDK_CONTEXT_H

struct _ASRSPI_CONTEXT {
    void *pCtx;
    int (*contextCheckDataType)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetSize)(struct _ASRSPI_CONTEXT *);
    char *(*contextGetLanguageVersionInfo)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetTS)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetAccuracy)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetRejection)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetGarbage)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrItem)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrWord)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrPron)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrFollower)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrStartWord)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetNbrStopWord)(struct _ASRSPI_CONTEXT *);
    unsigned long (*contextGetGarbageWordID)(struct _ASRSPI_CONTEXT *);
    unsigned char (*contextIsSpellingContext)(struct _ASRSPI_CONTEXT *);
    unsigned short *(*contextGetpLex)(struct _ASRSPI_CONTEXT *);
    unsigned long *(*contextGetpBeginOfWords)(struct _ASRSPI_CONTEXT *);
    unsigned short *(*contextGetpWordProp)(struct _ASRSPI_CONTEXT *);
    unsigned long *(*contextGetpSyntax)(struct _ASRSPI_CONTEXT *);
    unsigned char (*contextIsIsoWordContext)(struct _ASRSPI_CONTEXT *);
};

void FillContextV2VirtualTable(struct _ASRSPI_CONTEXT *pCtx);

#endif
