// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

/*
 * Sound, dude
 */

#include "Sound.h"
#include "GameConfig.h"
#ifdef PI_BUILD_WITH_OPENAL
	#include "AlAudioBackend.h"
#endif
#include "AudioBackend.h"
#include "Body.h"
#include "FileSystem.h"
#include "JobQueue.h"
#include "Pi.h"
#include "Player.h"
#include "SdlAudioBackend.h"
#include "utils.h"

#include <cassert>
#include <string>
#include <vector>

namespace Sound {

	static const unsigned int FREQ = 44100;
	static const double STREAM_IF_LONGER_THAN = 10.0;

	static AudioBackend *m_backend = nullptr;
	static std::vector<std::pair<std::string, Sample>> m_samples;

	void SetMasterVolume(const float vol)
	{
		m_backend->SetMasterVolume(vol);
	}

	float GetMasterVolume()
	{
		return m_backend->GetMasterVolume();
	}

	void SetSfxVolume(const float vol)
	{
		m_backend->SetSfxVolume(vol);
	}

	float GetSfxVolume()
	{
		return m_backend->GetSfxVolume();
	}

	void CalculateStereo(const Body *b, float vol, float *volLeftOut, float *volRightOut)
	{
		vector3d pos(0.0);

		if (b != Pi::player) {
			pos = b->GetPositionRelTo(Pi::player);
			pos = pos * Pi::player->GetOrient();
		}

		const float len = pos.Length();
		if (!is_zero_general(len)) {
			vol = vol / (0.002 * len);
			double dot = pos.Normalized().x * vol;

			(*volLeftOut) = vol * (2.0f - (1.0 + dot));
			(*volRightOut) = vol * (1.0 + dot);
		} else {
			(*volLeftOut) = (*volRightOut) = vol;
		}
		(*volLeftOut) = Clamp((*volLeftOut), 0.0f, 1.0f);
		(*volRightOut) = Clamp((*volRightOut), 0.0f, 1.0f);
	}

	void BodyMakeNoise(const Body *b, const char *sfx, float vol)
	{
		m_backend->BodyMakeNoise(b, sfx, vol);
	}

	void PlaySfx(const char *fx, const float volume_left, const float volume_right, const Op op)
	{
		m_backend->Play(fx, volume_left, volume_right, op);
	}

	void DestroyAllEvents()
	{
		m_backend->DestroyAllEvents();
	}

	void DestroyAllEventsExceptMusic()
	{
		m_backend->DestroyAllEventsExceptMusic();
	}

	static std::pair<std::string, Sample> load_sound(const std::string &basename, const std::string &path, bool is_music)
	{
		PROFILE_SCOPED()
		if (!ends_with_ci(basename, ".ogg")) return {};

		Sample sample;
		OggVorbis_File oggv;

		RefCountedPtr<FileSystem::FileData> oggdata = FileSystem::gameDataFiles.ReadFile(path);
		if (!oggdata) {
			Error("Could not read '%s'", path.c_str());
		}
		OggFileDataStream datastream(oggdata);
		oggdata.Reset();
		if (ov_open_callbacks(&datastream, &oggv, 0, 0, OggFileDataStream::CALLBACKS) < 0) {
			Error("Vorbis could not understand '%s'", path.c_str());
		}
		struct vorbis_info *info;
		info = ov_info(&oggv, -1);

		if ((static_cast<unsigned int>(info->rate) != FREQ) && (static_cast<unsigned int>(info->rate) != (FREQ >> 1))) {
			Error("Vorbis file %s is not %dHz or %dHz. Bad!", path.c_str(), FREQ, FREQ >> 1);
		}
		if ((info->channels < 1) || (info->channels > 2)) {
			Error("Vorbis file %s is not mono or stereo. Bad!", path.c_str());
		}

		const Sint64 num_samples = ov_pcm_total(&oggv, -1);
		// since samples are 16 bits we have:

		sample.buf_len = num_samples * info->channels;
		sample.channels = info->channels;
		sample.samplerate = info->rate;
		sample.path = path;

		const float seconds = num_samples / float(info->rate);
		//Output("%f seconds\n", seconds);

		// immediately decode and store as raw sample if short enough
		if (seconds < STREAM_IF_LONGER_THAN) {
			sample.buf.resize(sample.buf_len);

			int i = 0;
			for (;;) {
				int music_section;
				int amt = ov_read(&oggv, reinterpret_cast<char *>(sample.buf.data()) + i,
					2 * sample.buf_len - i, 0, 2, 1, &music_section);
				i += amt;
				if (amt == 0) break;
			}
		}

		ov_clear(&oggv);

		if (is_music) {
			sample.isMusic = true;
			// music keyed by pathname minus (datapath)/music/ and extension
			return { path.substr(0, path.size() - 4), sample };
		} else {
			sample.isMusic = false;
			// sfx keyed by basename minus the .ogg
			return { basename.substr(0, basename.size() - 4), sample };
		}
	}

	class LoadSoundJob : public Job {
	public:
		LoadSoundJob(const std::string &directory, bool isMusic) :
			m_directory(directory),
			m_isMusic(isMusic)
		{
		}

		void OnRun() override
		{
			PROFILE_SCOPED()
			// TODO: this is *probably* thread-safe, but FileSystem could use some further evaluation
			for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, m_directory, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
				const FileSystem::FileInfo &info = files.Current();
				assert(info.IsFile());
				std::pair<std::string, Sample> result = load_sound(info.GetName(), info.GetPath(), m_isMusic);
				m_loadedSounds.emplace(result);
			}
		}

		void OnFinish() override
		{
			for (auto &pair : m_loadedSounds) {
				m_samples.emplace_back(pair.first, pair.second);
				m_backend->AddSample(pair.first, std::move(pair.second));
			}
		}

	private:
		std::string m_directory;
		bool m_isMusic;
		std::map<std::string, Sample> m_loadedSounds;
	};

	BackendFlags GetAvailableBackends()
	{
		BackendFlags backends = AudioBackend_SDL;
#ifdef PI_BUILD_WITH_OPENAL
		backends |= AudioBackend_OpenAL;
#endif
		return backends;
	}

	BackendId GetBackendId()
	{
		return m_backend->GetId();
	}

	bool Init(BackendId backend)
	{
		PROFILE_SCOPED()
		if (m_backend != nullptr) {
			if (backend == m_backend->GetId()) {
				DestroyAllEvents();
				return true;
			} else {
				Uninit();
			}
		}

		try {
			switch (backend) {
#ifdef PI_BUILD_WITH_OPENAL
			case AudioBackend_OpenAL:
				try {
					m_backend = new AlAudioBackend();
					break;
				} catch (...) {
					Output("Could not initialize OpenAL audio backend, falling back to default");
				}
#endif
			default:
				m_backend = new SdlAudioBackend();
				break;
			}
		} catch (...) {
			Error("Could not initialize backend %u", backend);
			return false;
		}

		if (m_samples.empty()) {
			// load all the wretched effects
			Pi::GetApp()->GetAsyncStartupQueue()->Order(new LoadSoundJob("sounds", false));

			//I'd rather do this in MusicPlayer and store in a different map too, this will do for now
			Pi::GetApp()->GetAsyncStartupQueue()->Order(new LoadSoundJob("music", true));
		} else {
			for (auto sample_copy : m_samples) {
				m_backend->AddSample(sample_copy.first, std::move(sample_copy.second));
			}
		}

		SetMasterVolume(Pi::config->Float("MasterVolume"));
		SetSfxVolume(Pi::config->Float("SfxVolume"));
		EnableBinaural(Pi::config->Int("BinauralRendering"));
		if (Pi::config->Int("SfxMuted")) Sound::SetSfxVolume(0.f);

		Pause(Pi::config->Int("MasterMuted"));

		return true;
	}

	void Uninit()
	{
		if (m_backend == nullptr) {
			return;
		}

		DestroyAllEvents();
		delete m_backend;
		m_backend = nullptr;
	}

	void Pause(int on)
	{
		m_backend->Pause(on);
	}

	void Event::Play(const char *fx, float volume_left, float volume_right, Op op)
	{
		Stop();
		eid = m_backend->Play(fx, volume_left, volume_right, op);
	}

	void Event::PlayMusic(const char *fx, float volume, float fadeDelta, bool repeat, Event *fadeOut)
	{
		// The FadeOut, Stop, PlayMusicSample & VolumeAnimate calls perform
		// five mutex lock operations. These could be reduced to a single
		// lock/unlock if this were re-written.

		if (fadeOut) {
			fadeOut->FadeOut(fadeDelta);
		}
		Stop();
		float start = fadeDelta ? 0.0f : volume;
		eid = m_backend->Play(fx, start, start, repeat ? Sound::OP_REPEAT : 0);
		if (fadeDelta) {
			VolumeAnimate(volume, volume, fadeDelta, fadeDelta);
		}
	}

	bool Event::Stop()
	{
		return m_backend->EventStop(eid);
	}

	bool Event::IsPlaying() const
	{
		return m_backend->IsEventPlaying(eid);
	}

	bool Event::SetOp(Op op)
	{
		return m_backend->EventSetOp(eid, op);
	}

	bool Event::VolumeAnimate(const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2)
	{
		return m_backend->EventVolumeAnimate(eid, targetVol1, targetVol2, dv_dt1, dv_dt2);
	}

	bool Event::SetVolume(const float vol_left, const float vol_right)
	{
		return m_backend->EventSetVolume(eid, vol_left, vol_right);
	}

	bool Event::FadeOut(float dv_dt, Op op)
	{
		bool found = m_backend->EventVolumeAnimate(eid, 0.0f, 0.0f, dv_dt, dv_dt);
		if (found)
			m_backend->EventSetOp(eid, op | Sound::OP_STOP_AT_TARGET_VOLUME);
		return found;
	}

	const std::vector<std::string> GetMusicFiles()
	{
		std::vector<std::string> keys;
		for (const auto& sample : m_samples)
		{
			keys.push_back(sample.first);
		}
		return keys;
	}

	void Update(float delta_t)
	{
		m_backend->Update(delta_t);
	}

	bool IsBinauralSupported()
	{
		return m_backend->IsBinauralSupported();
	}

	void EnableBinaural(bool enabled)
	{
		m_backend->EnableBinaural(enabled);
	}

} /* namespace Sound */
