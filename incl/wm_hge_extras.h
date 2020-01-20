enum class wmHwndState : std::underlying_type_t<hgeBoolState> {
	EXTENDED_START = static_cast<std::underlying_type_t<hgeBoolState>>
	(hgeBoolState::HGE_SHOWSPLASH), // 7
	WM_WINDOWCAPTION = 30,
	WM_ACCEPTFILES = 31,
	WM_FILEDROPFUNC = 32,
	WM_NOTIFYFUNC = 33,
};
