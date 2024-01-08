#include <me/os/IEnvironment.h>

namespace mewos
{
	class Environment : public me::os::IEnvironment
	{
	public:
		std::string GetName() const override;
		std::string GetPlatform() const override;
	};
}