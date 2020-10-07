/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.

#ifndef FSAPI_FILEINFO_HPP_
#define FSAPI_FILEINFO_HPP_

#include <sos/link/types.h>

#include "var/StringView.hpp"

#if !defined __link
#include <unistd.h>
#define MCU_INT_CAST(var) ((void *)(u32)var)
#include <fcntl.h>
#else
#undef fileno
#define MCU_INT_CAST(var) ((void *)(u64)var)
#endif


namespace fs {

class FileInfoFlags {
public:
  enum class PermissionFlags {
    none = 0,
    public_execute = 0001,
    public_write = 0002,
    public_read = 0004,
    group_execute = 0010,
    group_write = 0020,
    group_read = 0040,
    owner_execute = 0100,
    owner_write = 0200,
    owner_read = 0400,
  };

  enum OpenFlags {
    null = 0,
    read_only /*! Open as read-only */ = LINK_O_RDONLY,
    write_only /*! Open as write-only */ = LINK_O_WRONLY,
    create /*! Create when opening (files) */ = LINK_O_CREAT,
    truncate /*! Truncate when opening (files) */ = LINK_O_TRUNC,
    append /*! Append when opening (files)*/ = LINK_O_APPEND,
    exclusive /*! Create exclusively (files) */ = LINK_O_EXCL,
    read_write /*! Open as read-write */ = LINK_O_RDWR,
    non_block /*! Open as non-blocking */ = LINK_O_NONBLOCK,
    no_delay /*! Open as non-blocking */ = LINK_O_NONBLOCK,
    access_mode /*! Access mode mask */ = LINK_O_ACCMODE
  };

  enum class AccessFlags {
    null = 0,
    read_ok /*! Check if read access is allowed */ = LINK_R_OK,
    write_ok /*! Check if write access is allowed */ = LINK_W_OK,
    execute_ok /*! Check if execute access is allowed */ = LINK_X_OK,
    file_ok /*! Check if file exists */ = LINK_F_OK
  };

  enum class TypeFlags {
    null = 0,
    mask /*! Mode format mask */ = LINK_S_IFMT,
    file_socket /*! Mode Socket mask */ = LINK_S_IFSOCK,
    regular /*! Mode regular file value */ = LINK_S_IFREG,
    block /*! Mode block device value */ = LINK_S_IFBLK,
    character /*! Mode character device value */ = LINK_S_IFCHR,
    directory /*! Mode directory value */ = LINK_S_IFDIR,
    fifo /*! Mode FIFO value */ = LINK_S_IFDIR,
    symbolic_link /*! Mode symbolic link value */ = LINK_S_IFLNK
  };
};

API_OR_NAMED_FLAGS_OPERATOR(FileInfoFlags, OpenFlags)
API_OR_NAMED_FLAGS_OPERATOR(FileInfoFlags, PermissionFlags)
API_OR_NAMED_FLAGS_OPERATOR(FileInfoFlags, AccessFlags)
API_OR_NAMED_FLAGS_OPERATOR(FileInfoFlags, TypeFlags)

class Permissions : public FileInfoFlags {
public:
  explicit Permissions(int mode = 0666) {
    m_permissions = static_cast<PermissionFlags>(mode);
  }

  static Permissions all_access() { return Permissions(0777); }
  static Permissions read_only() { return Permissions(0444); }
  static Permissions write_only() { return Permissions(0222); }

  bool is_owner_read() const {
    return m_permissions & PermissionFlags::owner_read;
  }
  bool is_owner_write() const {
    return m_permissions & PermissionFlags::owner_write;
  }
  bool is_owner_execute() const {
    return m_permissions & PermissionFlags::owner_execute;
  }
  bool is_public_read() const {
    return m_permissions & PermissionFlags::public_read;
  }
  bool is_public_write() const {
    return m_permissions & PermissionFlags::public_write;
  }
  bool is_public_execute() const {
    return m_permissions & PermissionFlags::public_execute;
  }

  Permissions &set_owner_read() {
    m_permissions |= PermissionFlags::owner_read;
    return *this;
  }
  Permissions &set_owner_write() {
    m_permissions |= PermissionFlags::owner_write;
    return *this;
  }
  Permissions &set_owner_execute() {
    m_permissions |= PermissionFlags::owner_execute;
    return *this;
  }

  Permissions &set_public_read() {
    m_permissions |= PermissionFlags::public_read;
    return *this;
  }
  Permissions &set_public_write() {
    m_permissions |= PermissionFlags::public_write;
    return *this;
  }
  Permissions &set_public_execute() {
    m_permissions |= PermissionFlags::public_execute;
    return *this;
  }

  bool operator==(const Permissions &a) const {
    return a.m_permissions == m_permissions;
  }

  int permissions() const { return static_cast<int>(m_permissions); }

private:
  PermissionFlags m_permissions;
};

class FileFlags : public FileInfoFlags {
public:
  FileFlags() { m_flags = OpenFlags::null; }
  explicit FileFlags(OpenFlags flags) { m_flags = flags; }
  explicit FileFlags(int flags) { m_flags = static_cast<OpenFlags>(flags); }

  static FileFlags create() {
    return FileFlags(OpenFlags::create | OpenFlags::read_write);
  }
  static FileFlags create_append_write_only() {
    return FileFlags(
      OpenFlags::create | OpenFlags::write_only | OpenFlags::append);
  }
  static FileFlags create_truncate() {
    return FileFlags(
      OpenFlags::create | OpenFlags::read_write | OpenFlags::truncate);
  }
  static FileFlags append() {
    return FileFlags(OpenFlags::read_write | OpenFlags::append);
  }
  static FileFlags append_read_write() {
    return FileFlags(OpenFlags::read_write | OpenFlags::append);
  }
  static FileFlags read_write() { return FileFlags(OpenFlags::read_write); }
  static FileFlags read_only() { return FileFlags(OpenFlags::read_only); }
  static FileFlags write_only() { return FileFlags(OpenFlags::write_only); }
  static FileFlags append_write_only() {
    return FileFlags(OpenFlags::write_only | OpenFlags::append);
  }

  bool is_read_only() const { return access() == OpenFlags::read_only; }
  bool is_write_only() const { return access() == OpenFlags::write_only; }
  bool is_read_write() const { return access() == OpenFlags::read_write; }
  OpenFlags access() const {
    OpenFlags result = m_flags;
    return result &= OpenFlags::access_mode;
  }

  bool is_create() const { return m_flags & OpenFlags::create; }
  bool is_exclusive() const { return m_flags & OpenFlags::exclusive; }
  bool is_truncate() const { return m_flags & OpenFlags::truncate; }
  bool is_append() const { return m_flags & OpenFlags::append; }
  bool is_non_blocking() const { return m_flags & OpenFlags::non_block; }

  FileFlags &set_read_only() {
    clear_access();
    m_flags |= OpenFlags::read_only;
    return *this;
  }
  FileFlags &set_write_only() {
    clear_access();
    m_flags |= OpenFlags::write_only;
    return *this;
  }
  FileFlags &set_read_write() {
    clear_access();
    m_flags |= OpenFlags::read_write;
    return *this;
  }
  FileFlags &set_create() {
    m_flags |= OpenFlags::create;
    return *this;
  }
  FileFlags &set_truncate() {
    m_flags |= OpenFlags::truncate;
    return *this;
  }
  FileFlags &set_append() {
    m_flags |= OpenFlags::append;
    return *this;
  }
  FileFlags &set_exclusive() {
    m_flags |= OpenFlags::exclusive;
    return *this;
  }
  FileFlags &set_non_blocking() {
    m_flags |= OpenFlags::non_block;
    return *this;
  }

  int o_flags() const { return m_flags; }

private:
  void clear_access() { m_flags &= ~OpenFlags::access_mode; }

  OpenFlags m_flags = OpenFlags::null;
};

class Access : public FileInfoFlags {
public:
  explicit Access(
    AccessFlags access = AccessFlags::read_ok | AccessFlags::write_ok
                         | AccessFlags::file_ok | AccessFlags::execute_ok) {
    m_access = access;
  }

  bool is_read_ok() const { return (m_access & AccessFlags::read_ok); }
  bool is_write_ok() const { return (m_access & AccessFlags::write_ok); }
  bool is_file_ok() const { return (m_access & AccessFlags::file_ok); }
  bool is_execute_ok() const { return (m_access & AccessFlags::execute_ok); }

  Access &set_read_ok() {
    m_access |= AccessFlags::read_ok;
    return *this;
  }
  Access &set_write_ok() {
    m_access |= AccessFlags::write_ok;
    return *this;
  }
  Access &set_file_ok() {
    m_access |= AccessFlags::file_ok;
    return *this;
  }
  Access &set_execute_ok() {
    m_access |= AccessFlags::execute_ok;
    return *this;
  }

  AccessFlags o_access() const { return m_access; }

private:
  AccessFlags m_access;
};

/*! \brief Stat Class
 * \details The Stat class is basically
 * a wrapper of `struct stat` and provides
 * information on the type and size
 * of the file.
 *
 * The file can be any filesystem object including
 * a file, directory, or device.
 *
 *
 * ```
 * #include <sapi/sys.hpp>
 *
 * Info info = File::get_info("/home/test.txt");
 *
 * if( info.is_file() ){
 *   printf("Is a file\n");
 * } else if( info.is_directory() ){
 *   printf("Is a directory\n");
 * }
 *
 * //grabbing the info of an already open file
 * File file;
 * file.open("/home/test.txt", File::RDONLY);
 * //grab the info from a file that is already open
 * info = file.get_info();
 * file.close();
 * ```
 *
 */
class FileInfo : public FileFlags {
public:
#if defined __link
  FileInfo(bool is_local = false);
  FileInfo(
    const struct FSAPI_LINK_STAT &st,
    bool is_local = false) // cppcheck-suppress[noExplicitConstructor]
    : m_stat(st), m_is_local(is_local) {}
#else
  /*! \details Constructs a new object.
   *
   * For the newly constructed object,
   * is_valid() returns false.
   *
   */
  FileInfo();

  /*! \details Constructs a new object
   * from the struct stat data provided.
   *
   */
  FileInfo(const struct stat &st) // cppcheck-suppress[noExplicitConstructor]
    : m_stat(st) {}
#endif

  /*! \details Returns true if the object is valid. */
  bool is_valid() const { return m_stat.st_mode != 0; }

  /*! \details Returns true if the file is a directory. */
  bool is_directory() const;

  /*! \details Returns true if the file is a regular file. */
  bool is_file() const;

  /*! \details Returns true if the file is a device (block or character). */
  bool is_device() const;

  /*! \details Returns true if the file is a block device. */
  bool is_block_device() const;

  /*! \details Returns true if the file is a character device. */
  bool is_character_device() const;

  /*! \details Returns true if the file is a socket. */
  bool is_socket() const;

  /*! \details Returns true if the file is a FIFO. */
  bool is_fifo() const;

  /*! \details Returns the size of the file in bytes.
   *
   * This method is only valid for regular files (ie is_file() must return
   * true).
   */
  u32 size() const;

  /*! \details Returns true if the file is executable. */
  bool is_executable() const;

  /*! \details Returns the file mode value. */
  Permissions permissions() const { return Permissions(m_stat.st_mode); }

  int owner() const { return m_stat.st_uid; }
  int group() const { return m_stat.st_gid; }

private:
#if defined __link
  struct FSAPI_LINK_STAT m_stat;
  bool m_is_local;
#else
  struct stat m_stat;
#endif
};

} /* namespace fs */

namespace printer {
class Printer;
Printer &operator<<(Printer &printer, const fs::FileInfo &a);
} // namespace printer

#endif /* FSAPI_FILEINFO_HPP_ */
