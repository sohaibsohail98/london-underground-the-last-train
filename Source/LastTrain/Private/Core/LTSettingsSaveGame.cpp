#include "Core/LTSettingsSaveGame.h"

namespace
{
	/** The field of view range a settings slider may offer. The low end is
		narrower than the coded 95 without being a scope, the high end is wide
		without turning the platform into a fisheye. Both are presentation limits,
		not balance ones: the weapon's own AimedFieldOfView is untouched. */
	constexpr float MinimumFieldOfView = 70.f;
	constexpr float MaximumFieldOfView = 120.f;
} // namespace

void FLTGameSettings::Sanitise()
{
	MasterVolume = FMath::Clamp(MasterVolume, 0.f, 1.f);
	FieldOfView = FMath::Clamp(FieldOfView, MinimumFieldOfView, MaximumFieldOfView);
}
