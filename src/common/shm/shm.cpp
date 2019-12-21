#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common/shm/shm.hpp"
#include "common/util/random_string.hpp"

namespace shm {
	static int open_shm(const std::string &name, int oflag) {
		int fd = ::shm_open(
				name.c_str(),
				oflag,
				S_IRUSR | S_IWUSR);

		if (fd < 0) {
			std::stringstream ss;
			ss << "Shared memory " << name << " could not be opened: " << ::strerror(errno);
			throw std::runtime_error{ss.str()};
		}

		return fd;
	}

	static void resize_shm(int fd) {
		if (::ftruncate(fd, initial_size) < 0) {
			std::stringstream ss;
			ss << "Resizing shared memory to " << initial_size << " B failed: " << ::strerror(errno);
			throw std::runtime_error{ss.str()};
		}
	}

	static void *mmap_and_close_shm(int fd) {
		auto memory = mmap(
				nullptr,
				initial_size,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_POPULATE | MAP_LOCKED,
				fd,
				0);

		if (memory == MAP_FAILED) {
			std::stringstream ss;
			ss << "mmapping shared memory failed: " << ::strerror(errno);
			throw std::runtime_error{ss.str()};
		}

		if (::close(fd) < 0) {
			std::stringstream ss;
			ss << "Closing shared memory file descriptor failed: " << ::strerror(errno);
			throw std::runtime_error{ss.str()};
		}
	
		return memory;
	}

	shm_t shm_t::create(const std::string &name) {
		int fd = open_shm(name, O_RDWR | O_CREAT | O_EXCL);
		resize_shm(fd);
		auto memory = mmap_and_close_shm(fd);
		return shm_t{name, memory};
	}

	shm_t shm_t::open(const std::string &name) {
		int fd = open_shm(name, O_RDWR);
		auto memory = mmap_and_close_shm(fd);
		return shm_t{name, memory};
	}

	void shm_t::unlink() {
		if (::shm_unlink(_name.c_str()) < 0) {
			std::stringstream ss;
			ss << "Unlinking shared memory region `" << _name << "` failed: " << ::strerror(errno);
			throw std::runtime_error{ss.str()};
		}
	}

	void *shm_t::memory() const {
		if (_memory == MAP_FAILED)
			throw std::runtime_error("Shared memory has been moved");

		return _memory;
	}

	shm_t::shm_t(const std::string &name, void *memory):
		_name(name),
		_memory(memory)
	{}

	shm_t::shm_t(shm_t&& that):
		_name(std::move(that._name)),
		_memory(that._memory)
	{
		that._memory = MAP_FAILED;
	}

	shm_t::~shm_t() {
		if (_memory != MAP_FAILED)
			::munmap(_memory, initial_size);
	}
}
