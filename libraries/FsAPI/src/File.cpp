/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
// Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc

#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include "chrono/ClockTimer.hpp"
#include "fs/File.hpp"

#include "local.h"

using namespace fs;

File::File(FSAPI_LINK_DECLARE_DRIVER) {
  m_fd = -1; // The file is not open
  set_keep_open(false);
}

File::File(
  var::StringView name,
  OpenMode flags FSAPI_LINK_DECLARE_DRIVER_LAST) {
  API_RETURN_IF_ERROR();
  open(name, flags);
}

File::~File() {
  if (is_keep_open() == false) {
    if (fileno() >= 0) {
      close();
    }
  }
}

File File::create(
  var::StringView path,
  IsOverwrite is_overwrite,
  Permissions perms FSAPI_LINK_DECLARE_DRIVER_LAST) {
  return File(FSAPI_LINK_INHERIT_DRIVER)
    .internal_create(path, is_overwrite, perms);
}

int File::interface_open(const char *path, int flags, int mode) const {
  return FSAPI_LINK_OPEN(driver(), path, flags, mode);
}

int File::interface_read(int fd, void *buf, int nbyte) const {
  return FSAPI_LINK_READ(driver(), fd, buf, nbyte);
}

int File::interface_write(int fd, const void *buf, int nbyte) const {
  return FSAPI_LINK_WRITE(driver(), fd, buf, nbyte);
}

int File::interface_ioctl(int fd, int request, void *argument) const {
  return FSAPI_LINK_IOCTL(driver(), fd, request, argument);
}

int File::interface_close(int fd) const {
  return FSAPI_LINK_CLOSE(driver(), fd);
}

int File::interface_fsync(int fd) const {
#if defined __link
  return 0;
#else
  return ::fsync(fd);
#endif
}

int File::interface_lseek(int fd, int offset, int whence) const {
  return FSAPI_LINK_LSEEK(driver(), fd, offset, whence);
}

File &
File::open(var::StringView path, OpenMode flags, Permissions permissions) {
  API_RETURN_VALUE_IF_ERROR(*this);
  if (m_fd != -1) {
    close(); // close first so the fileno can be changed
  }

  API_SYSTEM_CALL(
    path.cstring(),
    m_fd = interface_open(
      path.cstring(),
      flags.o_flags(),
      permissions.permissions()));
  return *this;
}

File &File::internal_create(
  var::StringView path,
  IsOverwrite is_overwrite,
  const Permissions &perms) {
  OpenMode flags = OpenMode::create();
  if (is_overwrite == IsOverwrite::yes) {
    flags.set_truncate();
  } else {
    flags.set_exclusive();
  }

  return open(path, flags, perms);
}

size_t File::size() const {
  // get current cursor
  API_RETURN_VALUE_IF_ERROR(0);
  int loc = location();
  API_SYSTEM_CALL("", interface_lseek(m_fd, 0, static_cast<int>(Whence::end)));
  API_RETURN_VALUE_IF_ERROR(0);
  size_t size = static_cast<size_t>(location());
  API_SYSTEM_CALL(
    "",
    interface_lseek(
      m_fd,
      loc,
      static_cast<int>(Whence::set))); // restore the cursor
  API_RETURN_VALUE_IF_ERROR(0);
  return size;
}

int File::fstat(struct FSAPI_LINK_STAT_STRUCT *st) {
  API_RETURN_VALUE_IF_ERROR(-1);
  return API_SYSTEM_CALL("", FSAPI_LINK_FSTAT(driver(), m_fd, st));
}

const File &File::readline(char *buf, int nbyte, int timeout, char term) const {
  API_RETURN_VALUE_IF_ERROR(*this);
  int t = 0;
  char c;
  int bytes_recv = 0;
  do {
    if (read(&c, 1).status().value() == 1) {
      *buf = c;
      buf++;
      bytes_recv++;
      if (c == term) {
        return *this;
      }
    } else {
      t++;
#if !defined __link
      chrono::wait(chrono::Milliseconds(1));
#endif
    }
  } while ((bytes_recv < nbyte) && (t < timeout));

  return *this;
}

File &File::close() {
  API_RETURN_VALUE_IF_ERROR(*this);
  if (m_fd >= 0) {
    API_SYSTEM_CALL("", interface_close(m_fd));
    m_fd = -1;
  }
  return *this;
}

const File &File::sync() const {
  API_RETURN_VALUE_IF_ERROR(*this);
#if defined __link
  if (driver()) {
    return *this;
  }
#endif
  if (m_fd >= 0) {
#if !defined __win32
    API_SYSTEM_CALL("", interface_fsync(m_fd));
#else
    ret = 0;
#endif
  }
  return *this;
}

const File &File::read(void *buf, int nbyte) const {
  API_RETURN_VALUE_IF_ERROR(*this);
  API_SYSTEM_CALL("", interface_read(m_fd, buf, nbyte));
  return *this;
}

const File &File::write(const void *buf, int nbyte) const {
  API_RETURN_VALUE_IF_ERROR(*this);
  API_SYSTEM_CALL("", interface_write(m_fd, buf, nbyte));
  return *this;
}

const File &File::seek(int location, Whence whence) const {
  API_RETURN_VALUE_IF_ERROR(*this);
  API_SYSTEM_CALL("", lseek(m_fd, location, static_cast<int>(whence)));
  return *this;
}

int File::fileno() const { return m_fd; }

int File::location() const {
  API_RETURN_VALUE_IF_ERROR(-1);
  return lseek(m_fd, 0, static_cast<int>(Whence::current));
}

int File::flags() const {
  API_RETURN_VALUE_IF_ERROR(-1);
#if defined __link
  return -1;
#else
  if (fileno() < 0) {
    API_SYSTEM_CALL("", -1);
    return status().value();
  }
  return _global_impure_ptr->procmem_base->open_file[m_fd].flags;
#endif
}

var::String File::gets(char term) const {
  API_RETURN_VALUE_IF_ERROR(var::String());
  char c;
  var::String result;
  do {
    if (read(var::View(c)).status().value() == 1) {
      result.append(c);
    } else {
      return result;
    }
  } while (c != term);

  return result;
}

const File &File::ioctl(int request, void *argument) const {
  API_RETURN_VALUE_IF_ERROR(*this);
  API_SYSTEM_CALL(
    "",
    interface_ioctl(m_fd, request, argument));
  return *this;
}

const File &File::write(File &source_file, const Write &options) const {
  API_RETURN_VALUE_IF_ERROR(*this);

  if (options.location() != static_cast<u32>(-1)) {
    seek(options.location(), Whence::set);
  }

  u32 size_processed = 0;

  const u32 file_size = options.size() == static_cast<u32>(-1)
                          ? source_file.size()
                          : options.size();

  if (file_size == 0) {
    if (options.progress_callback()) {
      options.progress_callback()->update(0, 100);
      options.progress_callback()->update(100, 100);
      options.progress_callback()->update(0, 0);
    }
    return *this;
  }

  const u32 read_buffer_size
    = options.page_size() ? options.page_size() : FSAPI_LINK_DEFAULT_PAGE_SIZE;

  u8 file_read_buffer[read_buffer_size];

  do {
    const u32 page_size = (file_size - size_processed < read_buffer_size)
                            ? file_size - size_processed
                            : read_buffer_size;

    const int bytes_read
      = source_file.read(file_read_buffer, page_size).status().value();

    if (bytes_read > 0) {
      if (options.transformer()) {
        const int transform_size
          = options.transformer()->get_output_size(page_size);
        u8 file_write_buffer[transform_size];
        const int bytes_to_write = options.transformer()->transform(
          var::Transformer::Transform()
            .set_input(var::View(file_read_buffer, page_size))
            .set_output(var::View(file_write_buffer, transform_size)));
        write(file_write_buffer, bytes_to_write);
      } else {
        write(file_read_buffer, bytes_read);
      }

      if (status().is_error()) {
        return *this;
      }

      if (bytes_read > 0) {
        size_processed += static_cast<u32>(bytes_read);
      } else if (bytes_read < 0) {
        API_SYSTEM_CALL("", -1);
        return *this;
      }
    }

    if (options.progress_callback()) {
      // abort the transaction
      if (
        options.progress_callback()->update(
          static_cast<int>(size_processed),
          static_cast<int>(file_size))
        == true) {
        options.progress_callback()->update(0, 0);
        API_SYSTEM_CALL("aborted", size_processed);
        return *this;
      }
    }

  } while ((source_file.status().value() > 0) && (file_size > size_processed));

  // this will terminate the progress operation
  if (options.progress_callback()) {
    options.progress_callback()->update(0, 0);
  }

  if ((source_file.status().is_error()) && (size_processed == 0)) {
    API_SYSTEM_CALL("", -1);
  }

  return *this;
}

DataFile::DataFile(fs::File &file_to_load) : FileAccess("") {
  m_location = 0;
  m_open_flags = OpenMode::append_read_write();
  API_RETURN_IF_ERROR();
  write(file_to_load, Write());
  seek(0);
  m_open_flags = OpenMode::read_write();
}

int DataFile::interface_read(int fd, void *buf, int nbyte) const {

  if (flags().is_write_only()) {
    return -1;
  }

  int size_ready = m_data.size_signed() - m_location;
  if (size_ready > nbyte) {
    size_ready = nbyte;
  }

  if (size_ready < 0) {
    // EOF
    return -1;
  }

  memcpy(buf, m_data.data_u8() + m_location, size_ready);

  m_location += size_ready;
  return size_ready;
}

int DataFile::interface_write(int fd, const void *buf, int nbyte) const {

  if (flags().is_read_only()) {
    return -1;
  }

  int size_ready = 0;
  if (flags().is_append()) {
    // make room in the m_data object for more bytes
    m_location = static_cast<int>(m_data.size());
    if (m_data.resize(m_data.size() + nbyte).status().is_error()) {
      return -1;
    }
    size_ready = nbyte;
  } else {
    // limit writes to the current size of the data
    if (static_cast<int>(m_data.size()) > m_location) {
      size_ready = m_data.size_signed() - m_location;
      if (size_ready > nbyte) {
        size_ready = nbyte;
      }
    } else {
      return -1;
    }
  }

  memcpy(m_data.data_u8() + m_location, buf, size_ready);

  m_location += size_ready;
  return size_ready;
}

int DataFile::interface_lseek(int fd, int offset, int whence) const {
  MCU_UNUSED_ARGUMENT(fd);
  switch (static_cast<Whence>(whence)) {
  case Whence::current:
    m_location += offset;
    break;
  case Whence::end:
    m_location = static_cast<int>(m_data.size());
    break;
  case Whence::set:
    m_location = offset;
    break;
  }

  if (m_location > static_cast<int>(size())) {
    m_location = static_cast<int>(m_data.size());
  } else if (m_location < 0) {
    m_location = 0;
  }

  return m_location;
}

int ViewFile::interface_read(int fd, void *buf, int nbyte) const {

  if (flags().is_write_only()) {
    return -1;
  }

  int size_ready = item().size() - m_location;
  if (size_ready > nbyte) {
    size_ready = nbyte;
  }

  if (size_ready < 0) {
    return -1;
  }

  memcpy(buf, item().to_const_u8() + m_location, size_ready);

  m_location += size_ready;
  return size_ready;
}

int ViewFile::interface_write(int fd, const void *buf, int nbyte) const {
  MCU_UNUSED_ARGUMENT(fd);

  if (flags().is_read_only()) {
    return -1;
  }

  if (item().is_read_only()) {
    return -1;
  }

  int size_ready = 0;
  if (flags().is_append()) {
    return -1;
  } else {
    // limit writes to the current size of the data
    size_ready = item().size() - m_location;
    if (size_ready > nbyte) {
      size_ready = nbyte;
    }
  }

  if (size_ready < 0) {
    return -1;
  }

  memcpy(item().to_u8() + m_location, buf, size_ready);

  m_location += size_ready;
  return size_ready;
}

int ViewFile::interface_lseek(int fd, int location, int whence) const {
  switch (static_cast<Whence>(whence)) {
  case Whence::current:
    m_location += location;
    break;
  case Whence::end:
    m_location = item().size_signed();
    break;
  case Whence::set:
    m_location = location;
    break;
  }

  if (m_location > item().size_signed()) {
    m_location = item().size_signed();
  } else if (m_location < 0) {
    m_location = 0;
  }

  return m_location;
}
