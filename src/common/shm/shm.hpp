#ifndef SHM_SHM_HPP
#define SHM_SHM_HPP

#include <string>

namespace shm {
	constexpr size_t initial_size = 1024 * 1024; // meh
	
	struct shm_t {
		/// Creates a new region of shared memory
		static shm_t create(const std::string &name);
		
		/// Opens an existing region of shared memory when given its name
		static shm_t open(const std::string &name);

		/// You can't copy the shared region
		shm_t(const shm_t&) = delete;

		/// Moves the shared memory region
		shm_t(shm_t&& that);

		/// Unmaps the shared memory region
		~shm_t();

		/// Unlinks the shared memory region
		void unlink();

		inline const std::string &name() const {
			return _name;
		}

		void *memory() const;

	private:
		shm_t(const std::string &name, void *memory);

		std::string _name;
		void *_memory;
	};
}

#endif
