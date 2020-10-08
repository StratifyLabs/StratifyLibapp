#ifndef FSAPI_SRC_LOCAL_H
#define FSAPI_SRC_LOCAL_H

#include <sos/link.h>

#if defined __link
#define FSAPI_LINK_OPEN(w, x, y, z) link_open(w, x, y, z)
#define FSAPI_LINK_IOCTL(w, x, y, z) link_ioctl(w, x, y, z)
#define FSAPI_LINK_RENAME(x, y, z) link_rename(x, y, z)
#define FSAPI_LINK_UNLINK(x, y) link_unlink(x, y)
#define FSAPI_LINK_LSEEK(w, x, y, z) link_lseek(w, x, y, z)
#define FSAPI_LINK_WRITE(w, x, y, z) link_write(w, x, y, z)
#define FSAPI_LINK_READ(w, x, y, z) link_read(w, x, y, z)
#define FSAPI_LINK_CLOSE(x, y) link_close(x, y)
#define FSAPI_LINK_STAT(x, y, z) link_stat(x, y, z)
#define FSAPI_LINK_FSTAT(x, y, z) link_fstat(x, y, z)
#define LINK_SET_DRIVER(x, y) x.set_driver(y)
#define LINK_DRIVER_ONLY driver()
#else
#define FSAPI_LINK_OPEN(w, x, y, z) ::open(x, y, z)
#define FSAPI_LINK_IOCTL(w, x, y, z) ::ioctl(x, y, z)
#define FSAPI_LINK_RENAME(x, y, z) ::rename(y, z)
#define FSAPI_LINK_UNLINK(x, y) ::remove(y)
#define FSAPI_LINK_LSEEK(w, x, y, z) ::lseek(x, y, z)
#define FSAPI_LINK_WRITE(w, x, y, z) ::write(x, y, z)
#define FSAPI_LINK_READ(w, x, y, z) ::read(x, y, z)
#define FSAPI_LINK_CLOSE(x, y) ::close(y)
#define FSAPI_LINK_STAT(x, y, z) ::stat(y, z)
#define FSAPI_LINK_FSTAT(x, y, z) ::fstat(y, z)
#define LINK_SET_DRIVER(x, y)
#define LINK_DRIVER_ONLY
#endif

#endif // FSAPI_SRC_LOCAL_H