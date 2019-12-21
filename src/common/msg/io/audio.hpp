#ifndef MSG_IO_PROCESS_HPP
#define MSG_IO_PROCESS_HPP

namespace msg::io {
	template <typename Ctx, typename T>
	void write_audio_arrays(const Ctx &ctx, VstInt32 num_samples, VstInt32 num_arrays, T **arrays) {
		// skip the arrays of arrays
		ctx.shm.template skip_data_array<T *>(num_arrays);

		// and then write the individual arrays
		for (int i = 0; i < num_arrays; i++)
			ctx.shm.write_data_array(arrays[i], num_samples);
	}

	template <typename Ctx, typename T>
	void map_audio_arrays(const Ctx &ctx, VstInt32 num_samples, VstInt32 num_arrays, T **&arrays) {
		// now map the array of arrays
		ctx.shm.map_data_array(arrays, num_arrays);
		
		// and now map the individual arrays
		for (int i = 0; i < num_arrays; i++)
			ctx.shm.map_data_array(arrays[i], num_samples);
	}

	template <typename Ctx, typename T>
	void skip_audio_arrays(const Ctx &ctx, VstInt32 num_samples, VstInt32 num_arrays, T **arrays) {
		// skip the array of arrays
		ctx.shm.template skip_data_array<T *>(num_arrays);

		// and skip all the individual arrays
		ctx.shm.template skip_data_array<T>(num_samples * num_arrays);
	}

	template <typename Ctx, typename T>
	void read_audio_arrays(const Ctx &ctx, VstInt32 num_samples, VstInt32 num_arrays, T **arrays) {
		// first we skip the array of arrays
		ctx.shm.template skip_data_array<T *>(num_arrays);

		// and then we copy data from the individual arrays
		for (int i = 0; i < num_arrays; i++)
			ctx.shm.read_data_array(arrays[i], num_samples);
	}
};

#endif
