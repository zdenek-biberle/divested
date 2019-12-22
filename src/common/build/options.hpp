#define DIVESTED_BUILD_OPTION_(x) #x
#define DIVESTED_BUILD_OPTION(x) DIVESTED_BUILD_OPTION_(x)

namespace build {
	constexpr const char *default_server_path = DIVESTED_BUILD_OPTION(DIVESTED_SERVER_PATH);
	constexpr const char *default_plugin_path = DIVESTED_BUILD_OPTION(DIVESTED_PLUGIN_PATH);
}
