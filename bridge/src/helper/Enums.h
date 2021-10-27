/*
 * Enums.h
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef ENUMS_H_
#define ENUMS_H_

enum GAME_TYPE {
	BRIDGE, PREFERANS
};

/* terms misere and whister (in russian вистующий) was taken from
 * english version of Wikipedia for preferans game
 * https://en.wikipedia.org/wiki/Preferans
 * */
enum STRING_ID {
	STRING_INVALID = -1,

	STRING_NORTH = 0,
	STRING_EAST = 1,
	STRING_SOUTH = 2,
	STRING_WEST = 3,
	STRING_NUMBER_OF_TRICKS = 4,
	STRING_TRICKS = 5,
	STRING_BRIDGE = 6,
	STRING_PREFERANS = 7,
	STRING_CLEAR = 8,
	STRING_DUMMY = 9,//NOT USED
	STRING_BEST_MOVE = 10,
	STRING_CONTRACT = 11,
	STRING_YES = 12,
	STRING_NO = 13,
	STRING_SAVED_BY_BRIDGE_STUDIO = 14,
	STRING_CLICK_TO_SWITCH_MOVE = 15,
	STRING_ABOUT = 16,
	STRING_J = 17,
	STRING_Q = 18,
	STRING_K = 19,
	STRING_A = 20,
	STRING_DOCUMENT_MODIFIED_WARNING_MESSAGE = 21,
	STRING_FIRST_SPLIT_FILE_NUMBER = 22,
	STRING_FROM_FILE = 23,
	STRING_ERRORS_WAS_FOUND_DURING_FILE_PROCEEDING = 24,
	STRING_THIS_FILE_IS_DAMAGED = 25,
	STRING_STORE_BEST_MOVE = 26, //Note items 26-29 should go in a row
	STRING_STORE_NUMBER_OF_TRICKS_IN_OPTIMAL_GAME = 27,
	STRING_BEFORE = 28,
	STRING_NEVER_SHOW_THIS_MESSAGE_AGAIN = 29,
	STRING_UNTITLED = 30,
	STRING_NEW_VERSION_FOUND = 31,
	STRING_TRICKS_OF_NORTH_AND_SOUTH = 32, //tooltip
	STRING_TRICKS_OF_WEST_AND_EAST = 33, //tooltip
	STRING_PROBLEM_DESCRIPTION = 34,
	STRING_ARROW = 35,
	STRING_MOVE_LEFT = 36,
	STRING_MOVE_RIGHT = 37,
	STRING_INSERT = 38,
	STRING_DELETE = 39,
	STRING_NT = 40,
	STRING_OK = 41,
	STRING_CANCEL = 42,
	STRING_RESTORE_DEFAULT = 43,
	STRING_FIND_BEST_MOVE = 44,
	STRING_STOP_SEARHING_BEST_MOVE = 45,
	STRING_CARDS_ORDER_INSIDE_SUIT = 46,
	STRING_RESIZE_MAIN_WINDOW = 47,
	STRING_MISERE = 48,
	STRING_DECK = 49,
	STRING_FILE_FILTER_BTS_PTS = 50,
	STRING_AFTER = 51,
	STRING_IMAGE_FILES = 52,
	STRING_DECLARER = 53,
	STRING_MISERE_PLAYER = 54,
	STRING_CATCHER = 55,
	STRING_WHISTER = 56,
	STRING_CLICK_TO_SWITCH_PLAYER = 57,//tooltip
	STRING_CLICK_TO_SWITCH_MISERE_PLAYER = 58,//tooltip
	STRING_TRICKS_OF_WHISTERS = 59, //tooltip
	STRING_TRICKS_OF_CATCHERS = 60, //tooltip
	STRING_TRICKS_OF_PLAYER = 61, //tooltip
	STRING_TRICKS_OF_MISERE_PLAYER = 62, //tooltip
	STRING_FILE_FILTER_PBN = 63,
	STRING_FILE_FILTER_DF = 64,
	STRING_FILE_FILTER_LANGUAGE = 65,
	STRING_FILE_FILTER_ALL = 66,
	STRING_FILE_FILTER_HTML = 67,
	STRING_VULNERABLE = 68,
	STRING_CONVERTER_HELP = 69,
	STRING_ADD = 70,
	/* all supported formats, cann't add extensions here because for save & load command
	 * use different types, in case of save should add html files
	 */
	STRING_FILE_FILTER_ALL_SUPPORTED = 71,
	STRING_CONTRACT_FROM_AUCTION_TAG_SHOULD_MATCH_WITH_CONTRACT_TAG = 72,
	STRING_DECLARER_FROM_AUCTION_TAG_SHOULD_MATCH_WITH_DECLARER_TAG = 73,
	STRING_DRAG_ABSENT_CARDS_HERE = 74,
	STRING_INVALID_DATE = 75,
	STRING_TO_ONE_FILE = 76,
	STRING_TO_MANY_FILES = 77,
	STRING_FILE = 78,
	STRING_ERROR = 79,
	STRING_WARNING_STORE_PREF_TO_DF_PBN = 80,
	STRING_NEW_DEAL = 81,
	STRING_RANDOM_DEAL = 82,
	STRING_QUANTITY = 83,
	STRING_TYPE = 84,
	STRING_WARNING_STORE_NO_CONTRACT_OR_NO_TRUMP_TO_DF_PBN = 85,
	STRING_ALL_DEALS = 86,
	STRING_CURRENT_DEAL = 87,
	STRING_SELECT_COLOR = 88,
	STRING_SELECT_IMAGE = 89,
	STRING_TOTAL = 90,
	STRING_POSITIONS = 91,
	STRING_TIME = 92,
	STRING_COPY_TO_CLIPBOARD = 93,
	STRING_ROTATE_BY_90_DEGREES = 94,
	STRING_CLICK_TO_ROTATE_BY_90_DEGREES = 95,
	STRING_FIRST_MOVE = 96,
	STRING_CLICK_TO_UNDO_MOVE = 97,//tooltip
	STRING_CLICK_TO_UNDO_MOVES = 98,//tooltip
	STRING_CLICK_TO_MAKE_MOVE = 99,//tooltip
	STRING_CLICK_TO_MAKE_MOVES = 100,//tooltip
	STRING_TRUMP=101,
	STRING_SPADES=102,
	STRING_HEARTS=103,
	STRING_DIAMONDS=104,
	STRING_CLUBS=105,
	STRING_PLAYERS_WITH_UNKNOWN_CARDS=106,
	STRING_LEFT=107,
	STRING_PLAYER_TRICKS=108,
	STRING_TRICKS1=109,
	STRING_THREAD,
	STRING_PER_SECOND,
	STRING_AVERAGE,
	STRING_VECTOR,
	STRING_RASTER,
	STRING_DECK_SELECTION_HELP,
	STRING_ARROW_SELECTION_HELP,
	STRING_THOUSANDS_SEPARATOR,
	STRING_WHIST,
	STRING_HALF_WHIST,
	STRING_ALL_PASS,
	STRING_WHIST_OPTION,
	STRING_PLAYERS,
	STRING_CONTRACTS_SCORING,
	STRING_CALCULATIONS,
	STRING_CONTRACTS_SCORING_HELP,
	STRING_OPTIONS,
	STRING_WHIST_OPTIONS_COMBO,
	STRING_EXPECTED_VALUE_TABLE,

	STRING_UNKNOWN_ERROR = 200,
	STRING_ERROR_DUPLICATE_TAG_FOUND,
	STRING_ERROR_CONTRACT_NOT_FOUND,
	STRING_ERROR_ONLEAD_NOT_FOUND,
	STRING_ERROR_LEAD_NOT_FOUND,
	STRING_ERROR_COMMENT_NOT_FOUND,
	STRING_ERROR_TOO_MANY_LEADING_CARDS,
	STRING_ERROR_COULD_NOT_OPEN_FILE,
	STRING_ERROR_INVALID_TAG_STRING,
	STRING_ERROR_INVALID_CARD,
	STRING_ERROR_DECLARER_TAG_NOT_FOUND,
	STRING_ERROR_UNRECOGNIZED_CARD_RANK,
	STRING_ERROR_UNRECOGNIZED_SUIT,
	STRING_ERROR_UNRECOGNIZED_PLAYER,
	STRING_ERROR_UNRECOGNIZED_CONTRACT,
	STRING_ERROR_UNRECOGNIZED_TRUMP,
	STRING_ERROR_INVALID_FILE_TYPE,
	STRING_ERROR_FOUND_TWO_CARD_OCCURENCE_IN_DEAL,
	STRING_ERROR_INVALID_DEAL,
	STRING_ERROR_CARD_IN_LIST_OF_MOVES_IS_FOUND_MORE_THAN_ONE_TIME,
	STRING_ERROR_INVALID_MOVE_FOUND,
	STRING_ERROR_TURNS_NOT_FOUND,
	STRING_ERROR_IMPOSSIBLE_TO_SAVE_PROBLEMS_LIST_TO_PBN_DF_FILE_BECAUSE_IT_HAS_ONLY_PREFERANS_PROBLEMS,
	STRING_ERROR_AUCTION_TAG_NUMBER_OF_CALLS_IN_ONE_ROW_SHOULD_BE_EQUALS_OR_LOWER_THAN_FOUR,
	STRING_ERROR_AUCTION_TAG_NUMBER_OF_CALLS_IS_LOWER_THAN_FOUR,
	STRING_ERROR_AUCTION_TAG_LAST_THREE_CALLS_SHOULD_BE_PASS,
	STRING_ERROR_AUCTION_TAG_MORE_THAH_THREE_PASS_IN_A_ROW_FOUND,
	STRING_ERROR_AUCTION_TAG_INVALID_CALL_STRING,
	STRING_ERROR_AUCTION_TAG_FOUND_CALL_WHICH_IS_LOWER_THAN_PREVIOUS_ONE,
	STRING_ERROR_AUCTION_TAG_INVALID_DOUBLE_CALL,
	STRING_ERROR_AUCTION_TAG_INVALID_REDOUBLE_CALL,
	STRING_ERROR_CARDS_SET_NOT_FOR_ALL_PLAYERS,
	STRING_ERROR_EMPTY_FILE,
	STRING_ERROR_PLAY_NOT_FOUND,
	STRING_ERROR_NUMBER_OF_TURNS_EXCEEDS_NUMBER_OF_FIRST_MOVES,
	STRING_ERROR_UNRECOGNIZED_ABSENT,
	STRING_ERROR_COULD_NOT_OPEN_FILE_FOR_WRITING,

	STRING_SIZE
};

enum MENU_ID {
	//helpers menu id's
	MENU_INVALID = 0,
	MENU_RECENT = 1,
	MENU_PROBLEM = 700, //top menu [700-719]
	MENU_ADDONS = 701,
	MENU_VIEW = 702,
	MENU_LANGUAGE_FIRST = 720, //language [720-799]
	//variable menu items
	MENU_GAME_TYPE = 1000,
	MENU_FIND_BEST_MOVE = 1001,
	MENU_CUSTOM_SKIN = 1002,

	//on insert/remove items in MENU_PROBLEM need to change Menu::m_firstRecentPosition
	MENU_NEW = 800,
	MENU_OPEN,
	MENU_SAVE,
	MENU_SAVE_AS,
	MENU_EDIT,
	MENU_EDIT_PROBLEM_LIST,
	MENU_EDIT_DESCRIPTION,
	MENU_PBN_EDITOR,
	MENU_ROTATE_CLOCKWISE,
	MENU_ROTATE_COUNTERCLOCKWISE,
	MENU_SAVE_HTML,
	MENU_SAVE_HTML_WITH_PICTURES,
	MENU_SAVE_IMAGE,
	//MENU_RECENT...
	MENU_EXIT,

	MENU_CONVERT,
	MENU_SOLVE_FOR_ALL_DECLARERS,
	MENU_CLEAR_DEAL,
	MENU_RANDOM_DEAL,
	//MENU_GAME_TYPE
	MENU_CALCULATOR,
	MENU_SOLVE_ALL_FOE,
	MENU_ROTATE_BY_90_DEGREES_CLOCKWISE,
	MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE,

	MENU_LAST_TRICK_GAME_ANLYSIS,
	MENU_ANIMATION,
	MENU_TOOLTIPS,
	MENU_TOTAL_TRICKS,
	MENU_PLAYER_TRICKS,
	MENU_SELECT_ARROW,
	MENU_SELECT_DECK,
	MENU_SELECT_FONT,
	MENU_SELECT_FONT_COLOR,
	MENU_SUITS_ORDER,

	MENU_ESTIMATE_NONE,
	MENU_ESTIMATE_BEST_LOCAL,
	MENU_ESTIMATE_BEST_TOTAL,
	MENU_ESTIMATE_ALL_LOCAL,
	MENU_ESTIMATE_ALL_TOTAL,
	MENU_SHOW_HTML_OPTIONS,
	MENU_SAVE_HTML_FILE_WITH_IMAGES,
	MENU_PREVIEW_HTML_FILE,
	MENU_SHOW_MODIFIED_WARNING,
	MENU_AUTOPLAY_SEQUENCE,
	MENU_SPLIT_EVERY_FILE,
	MENU_ONLY_ONE_INSTANCE,

	MENU_UNDOALL,
	MENU_UNDO,
	MENU_REDO,
	MENU_REDOALL,
	//MENU_FIND_BEST_MOVE

	//MENU_LANGUAGE_FIRST...
	MENU_OTHER_LANGUAGE,

	MENU_SKIN0,
	MENU_SKIN1,
	MENU_SKIN2,
	MENU_SKIN3,
	MENU_SKIN4,
	MENU_SKIN5,
	MENU_SKIN6,
	MENU_SKIN7,
	//MENU_CUSTOM_SKIN

	MENU_HOMEPAGE,
	MENU_ABOUT
};

enum TOOLBAR_BUTTON {
	TOOLBAR_BUTTON_FIND_BEST,
	TOOLBAR_BUTTON_UNDOALL,
	TOOLBAR_BUTTON_UNDO,
	TOOLBAR_BUTTON_REDO,
	TOOLBAR_BUTTON_REDOALL,

	TOOLBAR_BUTTON_SIZE
};

enum BUTTON_STATE {
	BUTTON_STATE_ENABLED, BUTTON_STATE_DISABLED, BUTTON_STATE_FIND_BEST_STOP
};

enum CLICABLE_REGION {
	CLICABLE_REGION_NOT_FOUND,
	CLICABLE_REGION_ARROW,
	CLICABLE_REGION_CAPTION,
	CLICABLE_REGION_CAPTION_HORIZONTAL_TRICKS,
	CLICABLE_REGION_CAPTION_VERTICAL_TRICKS,
	CLICABLE_REGION_ABSENT,
	CLICABLE_REGION_UNDO_MOVE,
	CLICABLE_REGION_UNDO_MOVES
};

//DEFAULT_EXTENSION array in Widget.cpp should match with enum FILE_TYPE
enum FILE_TYPE {
	FILE_TYPE_ANY,
	FILE_TYPE_DF,
	FILE_TYPE_HTML,
	FILE_TYPE_LANGUAGE,
	FILE_TYPE_PBN,
	FILE_TYPE_BRIDGE,
	FILE_TYPE_PREFERANS,
	FILE_TYPE_IMAGE,
	FILE_TYPE_ERROR
};

enum DEAL_STATE {
	DEAL_STATE_VALID, DEAL_STATE_NEW, DEAL_STATE_EMPTY,	//players has no cards
	DEAL_STATE_ERROR
};

enum CHOOSER_OPTION {
	CHOOSER_OPTION_DEFAULT,
	CHOOSER_OPTION_OPEN_ALL_SUPPORTED,
	CHOOSER_OPTION_ADD_ALL_SUPPORTED,
	CHOOSER_OPTION_OPEN_OR_ADD_ALL_SUPPORTED,
	CHOOSER_OPTION_SAVE_AS_MANY,
	CHOOSER_OPTION_SAVE_AS_SINGLE,
	CHOOSER_OPTION_CONVERTER_SAVE_ONE_FILE,
	CHOOSER_OPTION_CONVERTER_SAVE_MANY_FILES,
};

enum BUTTONS_DIALOG_TYPE {
	BUTTONS_DIALOG_NONE,
	BUTTONS_DIALOG_OK,
	BUTTONS_DIALOG_OK_CANCEL,
	BUTTONS_DIALOG_YES_NO_CANCEL,
	BUTTONS_DIALOG_CONVERTER,
	BUTTONS_DIALOG_OK_CANCEL_CLEAR,
	BUTTONS_DIALOG_OK_CANCEL_RESTORE_DEFAULT,

	BUTTONS_DIALOG_SIZE
};

enum MESSAGE_ICON_TYPE {
	MESSAGE_ICON_NONE, MESSAGE_ICON_MESSAGE, MESSAGE_ICON_ERROR
};

#endif /* ENUMS_H_ */
