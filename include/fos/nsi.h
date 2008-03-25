/*
  include/fos/nsi.h
  Copyright (C) 2008 Oleg Fedorov

  Named Server Interface's framework for FOS
*/

#ifndef _FOS_NSI_H
#define _FOS_NSI_H

#include <types.h>
#include <fos/fs.h>
#include <string.h>
#include <fos/message.h>

#ifdef __cplusplus
class nsi_t {
 private:
  message *msg;
  int (*method [MAX_METHODS_CNT]) (struct message *msg);
  
 public:
  nsi_t(char *bindpath);
  nsi_t(){
    msg = new message;
  }

  bool add(u32_t n, int (*method) (struct message *msg));
  void remove(u32_t n);
  void wait_message();
  void wait_message(u32_t timeout);

  struct {
    void * send_buf;
    size_t send_size;
    void * recv_buf;
    size_t recv_size;
    pid_t  pid;
    tid_t  tid;
    u32_t  flags;
  } std;
};
#endif

struct fos_nsi {
  struct message *msg;
  int (*method [MAX_METHODS_CNT]) (struct message *msg);

  struct {
    void * send_buf;
    size_t send_size;
    void * recv_buf;
    size_t recv_size;
    pid_t  pid;
    tid_t  tid;
    u32_t  flags;
    } std;
};

typedef struct fos_nsi fos_nsi;

fos_nsi * nsi_init(char *bindpath);
void nsi_exit(fos_nsi *nsi);
int  nsi_add_method(fos_nsi *nsi, u32_t n, int (*method) (struct message *msg));
void nsi_remove_method(fos_nsi *nsi, u32_t n);
void nsi_wait_message(fos_nsi *nsi);

#endif
