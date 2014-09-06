#include <windows.h>
#include <codecvt>
#include "../SDK/foobar2000.h"
#include "../helpers/helpers.h"

DECLARE_COMPONENT_VERSION("NowplayingTunes","1.0.0",NULL)
class CPlayback : public play_callback_static
{
public:
	virtual unsigned int get_flags(void);
	virtual void on_playback_new_track(metadb_handle_ptr track);
	virtual void on_playback_stop(play_control::t_stop_reason p_reason) {};
	virtual void on_playback_time(double p_time) {};
	virtual void on_playback_seek(double time) {};
	virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused) {}
	virtual void on_playback_pause(bool p_state) {}
	virtual void on_playback_edited(metadb_handle_ptr track) {}
	virtual void on_playback_dynamic_info(const file_info & p_info) {};
	virtual void on_playback_dynamic_info_track(const file_info & p_info) {};
	virtual void on_volume_change(float p_new_val) {}
};

static play_callback_static_factory_t<CPlayback> pbsf;

unsigned int CPlayback::get_flags(void)
{
	return flag_on_playback_new_track;
}

void CPlayback::on_playback_new_track(metadb_handle_ptr track)
{
	//�p�C�v����������
	// �p�C�v�ɐڑ�����
	HANDLE hPipe = CreateFile(TEXT("\\\\.\\pipe\\NowPlayingtunesSongPipe"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// �p�C�v�̐ؒf���K�؂ɍs���Ă��Ȃ��ꍇERROR_PIPE_BUSY�ƂȂ�
	int errorNo = GetLastError();
	if (errorNo == ERROR_PIPE_BUSY || hPipe == INVALID_HANDLE_VALUE) {
		// �G���[����
		console::info("[NowplayingTunes] Pipe connect error");
		return;
	}

	// ���^�f�[�^�\��
	file_info_impl info;
	track->get_info(info);
	//���M���镶����
	std::string send;
	if (info.meta_exists("TITLE")){
		send = info.meta_get("TITLE", 0);
	}
	send += "\n";
	if (info.meta_exists("ALBUM")){
		send += info.meta_get("ALBUM", 0);
	}
	send += "\n";
	if (info.meta_exists("ARTIST")){
		send += info.meta_get("ARTIST", 0);
	}
	send += "\n";
	if (info.meta_exists("ALBUM ARTIST")){
		send += info.meta_get("ALBUM ARTIST", 0);
	}
	send += "\n";
	if (info.meta_exists("TRACK")){
		send += info.meta_get("TRACK", 0);
	}
	send += "\n";
	if (info.meta_exists("GENRE")){
		send += info.meta_get("GENRE", 0);
	}
	send += "\n";
	if (info.meta_exists("COMPOSER")){
		send += info.meta_get("COMPOSER", 0);
	}
	send += "\n";
	console::info(send.c_str());
	//�ϊ�����
	const size_t newsizew = strlen(send.c_str()) + 1;
	size_t convertedChars = 0;
	wchar_t *wcstring = new wchar_t[newsizew];
	mbstowcs_s(&convertedChars, wcstring, newsizew, send.c_str(), _TRUNCATE);
	DWORD dwByte = 0;
	// �p�C�v�ɏ�������
	WORD Unicode = 0xfeff; // UNICODE BOM
	WriteFile(hPipe, &Unicode, 2, &dwByte, NULL);
	WriteFile(hPipe, wcstring, sizeof(wchar_t) * lstrlen(wcstring), &dwByte, NULL);
	//WriteFile(hPipe, &send, send.length(), &dwByte, NULL);
	CloseHandle(hPipe);
	free(wcstring);
}