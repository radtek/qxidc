#ifndef _FTP_H
#define _FTP_H

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>

#ifdef NOLFS
#define off64_t long
#endif

// ≤ª≤…”√openssl
#define NOSSL

#ifndef NOSSL
#include <openssl/ssl.h>
#endif

#define CONNECTTIMEOUT 60

using namespace std;

typedef int  (*FtpCallbackXfer)(long xfered, void *arg);
typedef int  (*FtpCallbackIdle)(void *arg);
typedef void (*FtpCallbackLog)(char *str, void* arg, bool out);

#ifndef NOSSL
typedef bool (*FtpCallbackCert)(void *arg, X509 *cert);
#endif

struct ftphandle 
{
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char *buf;
    int dir;
    ftphandle *ctrl;
    int cmode;
    struct timeval idletime;
    FtpCallbackXfer xfercb;
    FtpCallbackIdle idlecb;
    FtpCallbackLog logcb;
    void *cbarg;
    long xfered;
    long cbbytes;
    long xfered1;
    char response[256];
#ifndef NOSSL
    SSL* ssl;
    SSL_CTX* ctx;
    BIO* sbio;
    int tlsctrl;
    int tlsdata;
    FtpCallbackCert certcb;
#endif
    long offset;
    bool correctpasv;
};

class ftplib 
{
public:
    int m_timeout;

    enum accesstype
    {
        dir = 1,
        dirverbose,
        fileread,
        filewrite,
        filereadappend,
        filewriteappend
    }; 

    enum transfermode
    {
        ascii = 'A',
        image = 'I'
    };

    enum connmode
    {
        pasv = 1,
        port
    };

    enum fxpmethod
    {
        defaultfxp = 0,
        alternativefxp
    };

    enum dataencryption
    {
        unencrypted = 0,
        secure
    };

    ftplib();
    ~ftplib();
    char* LastResponse();
    int Connect(const char *host);
    int Login(const char *user, const char *pass);
    int Site(const char *cmd);
    int Raw(const char *cmd);
    int SysType(char *buf, int max);
    int Mkdir(const char *path);
    int Chdir(const char *path);
    int Cdup();
    int Rmdir(const char *path);
    int Pwd(char *path, int max);
    int Nlst(const char *outputfile, const char *path);
    int Dir(const char *outputfile, const char *path);
    int Size(const char *path, int *size, transfermode mode=ftplib::image);
    int ModDate(const char *path, char *dt, int max);
    int Get(const char *outputfile, const char *path, transfermode mode=ftplib::image, long offset = 0);
    int Put(const char *inputfile, const char *path, transfermode mode=ftplib::image, long offset = 0);
    int Rename(const char *src, const char *dst);
    int Delete(const char *path);
#ifndef NOSSL    
    int SetDataEncryption(dataencryption enc);
    int NegotiateEncryption();
    void SetCallbackCertFunction(FtpCallbackCert pointer);
#endif
    int Quit();
    void SetCallbackIdleFunction(FtpCallbackIdle pointer);
    void SetCallbackLogFunction(FtpCallbackLog pointer);
    void SetCallbackXferFunction(FtpCallbackXfer pointer);
    void SetCallbackArg(void *arg);
    void SetCallbackBytes(long bytes);
    void SetCorrectPasv(bool b) { mp_ftphandle->correctpasv = b; };
    void SetCallbackIdletime(int time);
    void SetConnmode(connmode mode);
    static int Fxp(ftplib* src, ftplib* dst, const char *pathSrc, const char *pathDst, transfermode mode, fxpmethod method);
    
    ftphandle* RawOpen(const char *path, accesstype type, transfermode mode);
    int RawClose(ftphandle* handle); 
    int RawWrite(void* buf, int len, ftphandle* handle);
    int RawRead(void* buf, int max, ftphandle* handle); 

    void CloseHandle();

    ftphandle* mp_ftphandle;
private:

    int FtpXfer(const char *localfile, const char *path, ftphandle *nControl, accesstype type, transfermode mode);
    int FtpOpenPasv(ftphandle *nControl, ftphandle **nData, transfermode mode, int dir, char *cmd);
    int FtpSendCmd(const char *cmd, char expresp, ftphandle *nControl);
    int FtpAcceptConnection(ftphandle *nData, ftphandle *nControl);
    int FtpOpenPort(ftphandle *nControl, ftphandle **nData, transfermode mode, int dir, char *cmd);
    int FtpRead(void *buf, int max, ftphandle *nData);
    int FtpWrite(void *buf, int len, ftphandle *nData);
    int FtpAccess(const char *path, accesstype type, transfermode mode, ftphandle *nControl, ftphandle **nData);
    int FtpClose(ftphandle *nData);
    
    int socket_wait(ftphandle *ctl);
    int readline(char *buf,int max,ftphandle *ctl);
    int writeline(char *buf, int len, ftphandle *nData);
    int readresp(char c, ftphandle *nControl);
    
    void ClearHandle();
    int CorrectPasvResponse(unsigned char *v);
};

#endif
