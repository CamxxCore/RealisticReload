#pragma once

static LPCTSTR en_US[]{
	"Reload"
};

static LPCTSTR fr_FR[]{
	u8"Recharger"
};

static LPCTSTR de_DE[]{
	u8"Nachladen"
};

static LPCTSTR it_IT[]{
	u8"Ricarica"
};

static LPCTSTR es_ES[]{
	u8"Recargar"
};

static LPCTSTR pt_BR[]{
	u8"Recarregar"
};

static LPCTSTR pl_PL[]{
	u8"Przeładowanie"
};

static LPCTSTR ru_RU[]{
	u8"Перезарядка оружия"
};

static LPCTSTR ko_KR[]{
	u8"재장전"
};

static LPCTSTR zh_CN[]{
	u8"補彈"
};

static LPCTSTR ja_JP[]{
	u8"リロード"
};

enum GlobalTextEntry_t
{
	RELOAD_TEXT
};

static LPCTSTR* langtext_array[11]{
	en_US,
	fr_FR,
	de_DE,
	it_IT,
	es_ES,
	pt_BR,
	pl_PL,
	ru_RU,
	ko_KR,
	zh_CN,
	ja_JP
};

inline LPCTSTR getConstString(int langugageId, GlobalTextEntry_t textId)
{
	return langtext_array[langugageId][textId];
}
