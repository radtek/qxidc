#include "ssh.h"

int Cssh::waitsocket(int socket_fd,LIBSSH2_SESSION *session)
{
  struct timeval timeout;
  int rc;
  fd_set fd;
  fd_set *writefd = NULL;
  fd_set *readfd  = NULL;
  int dir;

  timeout.tv_sec  = 10;
  timeout.tv_usec = 0;

  FD_ZERO(&fd);

  FD_SET(socket_fd,&fd);

  /* now make sure we wait in the correct direction */
  dir = libssh2_session_block_directions(session);

  if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
      readfd = &fd;

  if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
      writefd = &fd;

  rc = select(socket_fd+1,readfd,writefd,NULL,&timeout);

  return rc;
}

Cssh::Cssh()
{
  memset(m_hostip,0,sizeof(m_hostip));
  memset(m_username,0,sizeof(m_username));
  memset(m_password,0,sizeof(m_password));

  m_session = NULL;
  m_channel = NULL;

  m_sock = -1;
}

Cssh::~Cssh()
{
  ssh_close();
}

int Cssh::ssh_init(char *username,char *password,char *hostip,int port)
{
  memset(m_hostip,0,sizeof(m_hostip));
  memset(m_username,0,sizeof(m_username));
  memset(m_password,0,sizeof(m_password));

  strcpy(m_hostip,hostip);
  strcpy(m_username,username);
  strcpy(m_password,password);

  m_session = NULL;
  m_channel = NULL;

  m_port = port;
  m_sock = -1;

  return 0;
}

void Cssh::ssh_bindlog(CLogFile *logfile)
{
  m_logfile = logfile;
}

int Cssh::ssh_connect()
{
  if (m_logfile == NULL) return -1;

  if (strlen(m_hostip) == 0 )   { m_logfile->Write("ip地址不能为空。\n"); return -1; }
  if (strlen(m_username) == 0 ) { m_logfile->Write("用户名不能为空。\n"); return -1; }
  if (strlen(m_password) == 0 ) { m_logfile->Write("密码不能为空。\n");   return -1; }

  int rc;
  int type;
  size_t len;

  const char *fingerprint;
  LIBSSH2_KNOWNHOSTS *nh;

  m_sock = socket(AF_INET,SOCK_STREAM,0);
  sockin.sin_family = AF_INET;  
  sockin.sin_port = htons(m_port);  
  sockin.sin_addr.s_addr = inet_addr(m_hostip);  

  if (libssh2_init(0) != 0) 
  {
    m_logfile->Write("libssh2 init failed.\n");
    ssh_close();  
    return -1;
  }

  // 连接
  if (connect(m_sock,(sockaddr*)(&sockin),sizeof(sockaddr_in)) != 0)  
  {  
    m_logfile->Write("libssh2 connect failed.\n");
    ssh_close();  
    return -1;  
  }  

  // 会话
  m_session = libssh2_session_init();  
 
  if (!m_session)
  {
    m_logfile->Write("libssh2 session init failed.\n");
    ssh_close();  
    return -1; 
  }

  // 设置是否为非阻塞模式
  libssh2_session_set_blocking(m_session,0);

  while((rc = libssh2_session_handshake(m_session,m_sock)) == LIBSSH2_ERROR_EAGAIN);

  if (rc) 
  {
    m_logfile->Write("libssh2 session handshake failed.\n");
    ssh_close();  
    return -1;
  }

  nh = libssh2_knownhost_init(m_session);

  if(!nh) 
  {
    m_logfile->Write("libssh2 nh failed.\n");
    ssh_close();  
    return -1;
  }

  libssh2_knownhost_readfile(nh,"known_hosts",LIBSSH2_KNOWNHOST_FILE_OPENSSH);
  libssh2_knownhost_writefile(nh, "dumpfile",LIBSSH2_KNOWNHOST_FILE_OPENSSH);

  fingerprint = libssh2_session_hostkey(m_session,&len,&type);
  
  if(!fingerprint) 
  /*{
    struct libssh2_knownhost *host;
    int check = libssh2_knownhost_checkp(nh,m_hostip,m_port,fingerprint,len,LIBSSH2_KNOWNHOST_TYPE_PLAIN|LIBSSH2_KNOWNHOST_KEYENC_RAW,&host);

    //m_logfile->Write("Host check:%d,key:%s\n",check,(check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH)?host->key:"<none>");
  }
  else*/ 
  {
    m_logfile->Write("libssh2 hostkey failed.\n");
    ssh_close();  
    return -1;
  }

  libssh2_knownhost_free(nh);
 
  if (strlen(m_password) != 0 ) 
  {
    while((rc = libssh2_userauth_password(m_session,m_username,m_password)) == LIBSSH2_ERROR_EAGAIN);

    if (rc) 
    {
      m_logfile->Write("Authentication by password failed.\n");
      ssh_close();
      return -1;
    }
  }
  else  
  {
    while((rc = libssh2_userauth_publickey_fromfile(m_session,m_username,
                                                         "/home/user/"
                                                         ".ssh/id_rsa.pub",
                                                         "/home/user/"
                                                         ".ssh/id_rsa",
                                                         m_password)) ==
               LIBSSH2_ERROR_EAGAIN);
    if (rc) 
    {
      m_logfile->Write("Authentication by public key failed.\n");
      ssh_close();
      return -1;
    }
  }

  while((m_channel = libssh2_channel_open_session(m_session)) == NULL &&
           libssh2_session_last_error(m_session,NULL,NULL,0) == LIBSSH2_ERROR_EAGAIN)
  {
    waitsocket(m_sock,m_session);
  }

  if(m_channel == NULL )
  {
    m_logfile->Write("createchannel failed.\n");
    ssh_close();
    return -1;
  }

  return 0;
}

int Cssh::ssh_read(char *outbuffer)
{
  if (m_channel == NULL) return -1;

  char buffer[4096];
  memset(outbuffer,0,sizeof(outbuffer));
  memset(buffer,0,sizeof(buffer));

  while(TRUE)
  {
    int rc;

    do
    {
      memset(buffer,0,sizeof(buffer));
      rc = libssh2_channel_read(m_channel,buffer,sizeof(buffer));

      if( rc > 0 )
      {
        strcat(outbuffer,buffer);
      }
      else 
      {
        if (rc != LIBSSH2_ERROR_EAGAIN && rc != 0)
        {
           m_logfile->Write("libssh2_channel_read returned %d.\n",rc);
           ssh_close();
           return -1;
        }
      }
    }
    while(rc > 0);

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
      waitsocket(m_sock,m_session);
    }
    else
      break;
  }

  return 0;
}

int Cssh::ssh_write(char *cmdstr)
{
  if (m_channel == NULL) return -1;
          
  int rc;
  while((rc = libssh2_channel_exec(m_channel,cmdstr)) == LIBSSH2_ERROR_EAGAIN)
  {
    waitsocket(m_sock,m_session);
  }

  if(rc != 0) {ssh_close(); return -1;}

  return 0;
}

void Cssh::ssh_close()
{
  if (m_channel)  
  {  
    int rc; 

    while((rc = libssh2_channel_close(m_channel)) == LIBSSH2_ERROR_EAGAIN)
    {
      waitsocket(m_sock,m_session);
    }

    libssh2_channel_free(m_channel);
    m_channel = NULL;  
  }

  if(m_session)  
  {  
    libssh2_session_disconnect(m_session, "Bye bye, Thank you");  
    libssh2_session_free(m_session);  
    m_session = NULL;  
  }  

  if( m_sock != -1 )  
  {  
    close(m_sock);  
    m_sock = -1;  
  }  

  libssh2_exit();
}

