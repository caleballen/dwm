/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 4;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 1;   	/* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;   
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int user_bh            = 20;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const char *fonts[]          = { "Nimbus Sans:size=9:bold" };
static const char dmenufont[]       = "Nimbus Sans:size=9";

static const char col_dark_grey[]   = "#1c2121";
static const char col_grey[]        = "#504945";
static const char col_yellow[]      = "#fabd2f";
static const char col_cyan[]        = "#83a598";
static const char col_teal[]        = "#8ec07c";
static const char col_red[]         = "#fb4934";
static const char col_green[]       = "#b8bb26";
static const char col_purple[]      = "#d3869b";
static const char col_white[]       = "#ebdbb2";
static const char col_orange[]         = "#fe8019";

static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_grey, col_dark_grey, col_dark_grey },
	[SchemeSel]  = { col_dark_grey, col_cyan,  col_cyan  },
	[SchemeStatus]  = { col_white, col_dark_grey,  "#000000"  }, // Statusbar right {text,background,not used but cannot be empty}
	[SchemeTagsSel]  = { col_dark_grey, col_cyan,  "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
    [SchemeTagsNorm]  = { col_white, col_dark_grey,  "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
	[SchemeTagsEmpty] = { col_white, col_grey, "#000000" },
    [SchemeInfoSel]  = { col_white, col_dark_grey,  "#000000"  }, // infobar middle  selected {text,background,not used but cannot be empty}
    [SchemeInfoNorm]  = { col_white, col_dark_grey,  "#000000"  }, // infobar middle  unselected {text,background,not used but cannot be empty}
	[SchemeLayout] = { col_dark_grey, col_cyan, "#000000"},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_dark_grey, "-nf", col_white, "-sb", col_cyan, "-sf", col_white, NULL };
static const char *termcmd[]  = { "kitty", NULL };

static const char *volumemute[] = { "amixer", "-D", "pulse", "set", "Master", "1+", "toggle",  NULL };
static const char *volumeraise[] = { "amixer", "-D", "pulse", "sset", "Master", "5%+",  NULL };
static const char *volumelower[] = { "amixer", "-D", "pulse", "sset", "Master", "5%-",  NULL };

static const char *backlightraise[] = { "xbacklight", "-inc", "10",  NULL };
static const char *backlightlower[] = { "xbacklight", "-dec", "10",  NULL };

static const char *playerplaypause[] = { "playerctl", "play-pause",  NULL };
static const char *playernext[] = { "playerctl", "next",  NULL };
static const char *playerprevious[] = { "playerctl", "previous",  NULL };

static const char *screenshot[] = { "gnome-screenshot", "-i", NULL };
static const char *screenshot_clipboard[] = { "gnome-screenshot", "-ac", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_r,      spawn,          {.v = dmenucmd } },
	{ MODKEY,						XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_z,      zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_f,      togglefullscr,      {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ 0,							XF86XK_AudioMute,	spawn,	{.v = volumemute }},
	{ 0,							XF86XK_AudioRaiseVolume,	spawn,	{.v = volumeraise}},
	{ 0,							XF86XK_AudioLowerVolume,	spawn,	{.v = volumelower }},
	{ 0,							XF86XK_MonBrightnessUp,	spawn,	{.v = backlightraise }},
	{ 0,							XF86XK_MonBrightnessDown,	spawn,	{.v = backlightlower}},
	{ 0,							XF86XK_AudioPlay,	spawn,	{.v = playerplaypause}},
	{ 0,							XF86XK_AudioPause,	spawn,	{.v = playerplaypause}},
	{ 0,							XF86XK_AudioNext,	spawn,	{.v = playernext}},
	{ 0,							XF86XK_AudioPrev,	spawn,	{.v = playerprevious}},
	{ Mod1Mask|ControlMask,			XK_Down,	spawn,	{.v = playerplaypause}},
	{ Mod1Mask|ControlMask,			XK_Right,	spawn,	{.v = playernext}},
	{ Mod1Mask|ControlMask,			XK_Left,	spawn,	{.v = playerprevious}},
	{ 0,							XK_Print,	spawn,	{.v = screenshot}},
	{ MODKEY|ShiftMask,				XK_s,		spawn,	{.v = screenshot_clipboard}},
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_e,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

