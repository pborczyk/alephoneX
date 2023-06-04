#ifndef __SOUND_PLAYER_H
#define __SOUND_PLAYER_H

#include "AudioPlayer.h"
#include "SoundFile.h"
#include "sound_definitions.h"

struct SoundStereo {

	bool is_panning;
	float gain_global;
	float gain_left;
	float gain_right;

	bool operator==(const SoundStereo& other) const {
		return std::tie(is_panning, gain_global, gain_right, gain_left) == std::tie(other.is_panning, other.gain_global, other.gain_right, other.gain_left);
	}

	bool operator!=(const SoundStereo& other) const {
		return !(*(this) == other);
	}
};

struct SoundParameters {
	short identifier = NONE; //Identifier of the sound
	short source_identifier = NONE; //Identifier of the source emitting the sound
	float pitch = 1;
	bool loop = false; //for now it will only be used by sound (musics work differently but should use this tbh)
	bool local = true; //if false it will use source_location3d to position sound (3D sounds)
	bool _is_for_rewind = false; //internal flag to know if those parameters must be loaded when rewinding sound or not
	uint16_t permutation = 0;
	uint16_t obstruction_flags = 0;
	sound_behavior behavior = _sound_is_normal;
	world_location3d source_location3d = {};
	world_location3d* dynamic_source_location3d = nullptr;
	SoundStereo stereo_parameters = {}; //2D panning
	uint16_t flags = 0;
};

struct SoundBehavior {
	float distance_reference;
	float distance_max;
	float rolloff_factor;
	float max_gain;
};

struct Sound {
	SoundInfo header;
	SoundData data;
};

class SoundPlayer : public AudioPlayer {
public:
	SoundPlayer(const Sound sound, SoundParameters parameters); //Must not be used outside OpenALManager (public for make_shared)
	void UpdateParameters(SoundParameters parameters) { this->parameters.Store(parameters); }
	short GetIdentifier() const override { return parameters.Get().identifier; }
	short GetSourceIdentifier() const override { return parameters.Get().source_identifier; }
	SoundParameters GetParameters() const { return parameters.Get(); }
	static float Simulate(const SoundParameters soundParameters);
	float GetPriority() const override { return Simulate(parameters.Get()); }
private:

	struct SoundTransition {
		uint32_t last_update_tick = 0;
		float current_volume = 0;
		bool allow_transition = false;
	};

	void Rewind() override;
	int GetNextData(uint8* data, int length) override;
	int LoopManager(uint8* data, int length);
	std::pair<bool, bool> SetUpALSourceIdle() override;
	bool SetUpALSourceInit() override;
	void SetUpALSource3D() const;
	bool CanRewindSound(int baseTick) const { return baseTick + rewind_time < GetCurrentTick(); }
	bool LoadParametersUpdates() override;
	void AskRewind(SoundParameters soundParameters, const Sound& sound);
	void ComputeVolumeForTransition(float targetVolume);
	AtomicStructure<Sound> sound;
	AtomicStructure<SoundParameters> parameters;
	SoundParameters rewind_parameters;
	SoundTransition sound_transition;
	uint32_t data_length;
	uint32_t current_index_data = 0;
	uint32_t start_tick;

	static constexpr int rewind_time = 83;
	static constexpr float smooth_volume_transition_threshold = 0.1f;
	static constexpr int smooth_volume_transition_time_ms = 300;

	static constexpr SoundBehavior sound_behavior_parameters[] = {
		{0.5, 5, 1, 1},
		{2.5, 15, 1.7, 1},
		{3, 20, 1.2, 1}
	};

	static constexpr SoundBehavior sound_obstruct_behavior_parameters[] = {
		{0.5, 3, 2, 0.3},
		{2.5, 9, 3.4, 0.5},
		{3, 12, 2.4, 0.75}
	};

	friend class OpenALManager;
};

#endif