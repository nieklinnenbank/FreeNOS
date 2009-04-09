/* Generated file. Do not edit. */

static teken_state_t	teken_state_3;
static teken_state_t	teken_state_5;
static teken_state_t	teken_state_2;
static teken_state_t	teken_state_1;
static teken_state_t	teken_state_6;
static teken_state_t	teken_state_7;
static teken_state_t	teken_state_8;
static teken_state_t	teken_state_4;

/* '^[[>' */
static void
teken_state_3(teken_t *t, teken_char_t c)
{

	if (teken_state_numbers(t, c))
		return;

	switch (c) {
	case 'c': /* DA2: Secondary Device Attributes */
		teken_subr_secondary_device_attributes(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	default:
		teken_printf("Unsupported sequence in teken_state_3: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '' */
static void
teken_state_init(teken_t *t, teken_char_t c)
{

	switch (c) {
	case '\x1B':
		teken_state_switch(t, teken_state_1);
		return;
	default:
		teken_subr_regular_character(t, c);
		break;
	}
}

/* '^[[?' */
static void
teken_state_5(teken_t *t, teken_char_t c)
{

	if (teken_state_numbers(t, c))
		return;

	switch (c) {
	case 'h': /* DECSM: Set DEC mode */
		teken_subr_set_dec_mode(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'l': /* DECRM: Reset DEC mode */
		teken_subr_reset_dec_mode(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'n': /* DSR: Device Status Report */
		teken_subr_device_status_report(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	default:
		teken_printf("Unsupported sequence in teken_state_5: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '^[[' */
static void
teken_state_2(teken_t *t, teken_char_t c)
{

	if (teken_state_numbers(t, c))
		return;

	switch (c) {
	case '@': /* ICH: Insert character */
		teken_subr_insert_character(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'A': /* CUU: Cursor Up */
		teken_subr_cursor_up(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'B': /* CUD: Cursor Down */
		teken_subr_cursor_down(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'C': /* CUF: Cursor Forward */
		teken_subr_cursor_forward(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'D': /* CUB: Cursor Backward */
		teken_subr_cursor_backward(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'E': /* CNL: Cursor Next Line */
		teken_subr_cursor_next_line(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'F': /* CPL: Cursor Previous Line */
		teken_subr_cursor_previous_line(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'G': /* HPA: Horizontal Position Absolute */
		teken_subr_horizontal_position_absolute(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'H': /* CUP: Cursor Position */
		teken_subr_cursor_position(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0], (t->t_curnum < 2 || t->t_nums[1] == 0) ? 1 : t->t_nums[1]);
		break;
	case 'I': /* CHT: Cursor Forward Tabulation */
		teken_subr_cursor_forward_tabulation(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'J': /* ED: Erase display */
		teken_subr_erase_display(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'K': /* EL: Erase line */
		teken_subr_erase_line(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'L': /* IL: Insert line */
		teken_subr_insert_line(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'M': /* DL: Delete line */
		teken_subr_delete_line(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'P': /* DC: Delete character */
		teken_subr_delete_character(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'S': /* SU: Pan Down */
		teken_subr_pan_down(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'T': /* SD: Pan Up */
		teken_subr_pan_up(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'X': /* ECH: Erase character */
		teken_subr_erase_character(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'Z': /* CBT: Cursor Backward Tabulation */
		teken_subr_cursor_backward_tabulation(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case '`': /* HPA: Horizontal Position Absolute */
		teken_subr_horizontal_position_absolute(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'a': /* CUF: Cursor Forward */
		teken_subr_cursor_forward(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'c': /* DA1: Primary Device Attributes */
		teken_subr_primary_device_attributes(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'd': /* VPA: Vertical Position Absolute */
		teken_subr_vertical_position_absolute(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'e': /* CUD: Cursor Down */
		teken_subr_cursor_down(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0]);
		break;
	case 'f': /* CUP: Cursor Position */
		teken_subr_cursor_position(t, (t->t_curnum < 1 || t->t_nums[0] == 0) ? 1 : t->t_nums[0], (t->t_curnum < 2 || t->t_nums[1] == 0) ? 1 : t->t_nums[1]);
		break;
	case 'g': /* TBC: Tab Clear */
		teken_subr_tab_clear(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'h': /* SM: Set Mode */
		teken_subr_set_mode(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'l': /* RM: Reset Mode */
		teken_subr_reset_mode(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'm': /* SGR: Set Graphic Rendition */
		teken_subr_set_graphic_rendition(t, t->t_curnum, t->t_nums);
		break;
	case 'n': /* CPR: Cursor Position Report */
		teken_subr_cursor_position_report(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'r': /* DECSTBM: Set top and bottom margins */
		teken_subr_set_top_and_bottom_margins(t, t->t_curnum < 1 ? 0 : t->t_nums[0], t->t_curnum < 2 ? 0 : t->t_nums[1]);
		break;
	case 's': /* DECSC: Save cursor */
		teken_subr_save_cursor(t);
		break;
	case 'u': /* DECRC: Restore cursor */
		teken_subr_restore_cursor(t);
		break;
	case 'z': /* C25VTSW: Cons25 switch virtual terminal */
		teken_subr_cons25_switch_virtual_terminal(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case '>':
		teken_state_switch(t, teken_state_3);
		return;
	case '?':
		teken_state_switch(t, teken_state_5);
		return;
	case '=':
		teken_state_switch(t, teken_state_8);
		return;
	default:
		teken_printf("Unsupported sequence in teken_state_2: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '^[' */
static void
teken_state_1(teken_t *t, teken_char_t c)
{

	switch (c) {
	case '7': /* DECSC: Save cursor */
		teken_subr_save_cursor(t);
		break;
	case '8': /* DECRC: Restore cursor */
		teken_subr_restore_cursor(t);
		break;
	case '=': /* DECKPAM: Keypad application mode */
		teken_subr_keypad_application_mode(t);
		break;
	case '>': /* DECKPNM: Keypad numeric mode */
		teken_subr_keypad_numeric_mode(t);
		break;
	case 'D': /* IND: Index */
		teken_subr_index(t);
		break;
	case 'E': /* NEL: Next line */
		teken_subr_next_line(t);
		break;
	case 'H': /* HTS: Horizontal Tab Set */
		teken_subr_horizontal_tab_set(t);
		break;
	case 'M': /* RI: Reverse index */
		teken_subr_reverse_index(t);
		break;
	case 'P': /* DCS: Device Control String */
		teken_subr_device_control_string(t);
		break;
	case 'c': /* RIS: Reset to Initial State */
		teken_subr_reset_to_initial_state(t);
		break;
	case '\\': /* ST: String Terminator */
		teken_subr_string_terminator(t);
		break;
	case '[':
		teken_state_switch(t, teken_state_2);
		return;
	case '(':
		teken_state_switch(t, teken_state_6);
		return;
	case ')':
		teken_state_switch(t, teken_state_7);
		return;
	case '#':
		teken_state_switch(t, teken_state_4);
		return;
	default:
		teken_printf("Unsupported sequence in teken_state_1: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '^[(' */
static void
teken_state_6(teken_t *t, teken_char_t c)
{

	switch (c) {
	case '1': /* G0SCS1: G0 SCS US ASCII */
		teken_subr_g0_scs_us_ascii(t);
		break;
	case '2': /* G0SCS2: G0 SCS Special Graphics */
		teken_subr_g0_scs_special_graphics(t);
		break;
	case 'A': /* G0SCSA: G0 SCS UK National */
		teken_subr_g0_scs_uk_national(t);
		break;
	case 'B': /* G0SCSB: G0 SCS US ASCII */
		teken_subr_g0_scs_us_ascii(t);
		break;
	case '0': /* G0SCS0: G0 SCS Special Graphics */
		teken_subr_g0_scs_special_graphics(t);
		break;
	default:
		teken_printf("Unsupported sequence in teken_state_6: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '^[)' */
static void
teken_state_7(teken_t *t, teken_char_t c)
{

	switch (c) {
	case '0': /* G1SCS0: G1 SCS Special Graphics */
		teken_subr_g1_scs_special_graphics(t);
		break;
	case '1': /* G1SCS1: G1 SCS US ASCII */
		teken_subr_g1_scs_us_ascii(t);
		break;
	case '2': /* G1SCS2: G1 SCS Special Graphics */
		teken_subr_g1_scs_special_graphics(t);
		break;
	case 'A': /* G1SCSA: G1 SCS UK National */
		teken_subr_g1_scs_uk_national(t);
		break;
	case 'B': /* G1SCSB: G1 SCS US ASCII */
		teken_subr_g1_scs_us_ascii(t);
		break;
	default:
		teken_printf("Unsupported sequence in teken_state_7: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '^[[=' */
static void
teken_state_8(teken_t *t, teken_char_t c)
{

	if (teken_state_numbers(t, c))
		return;

	switch (c) {
	case 'F': /* C25ADFG: Cons25 set adapter foreground */
		teken_subr_cons25_set_adapter_foreground(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'G': /* C25ADBG: Cons25 set adapter background */
		teken_subr_cons25_set_adapter_background(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	case 'S': /* C25CURS: Cons25 set cursor type */
		teken_subr_cons25_set_cursor_type(t, t->t_curnum < 1 ? 0 : t->t_nums[0]);
		break;
	default:
		teken_printf("Unsupported sequence in teken_state_8: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}

/* '^[#' */
static void
teken_state_4(teken_t *t, teken_char_t c)
{

	switch (c) {
	case '3': /* DECDHL: Double Height Double Width Line Top */
		teken_subr_double_height_double_width_line_top(t);
		break;
	case '4': /* DECDHL: Double Height Double Width Line Bottom */
		teken_subr_double_height_double_width_line_bottom(t);
		break;
	case '5': /* DECSWL: Single Height Single Width Line */
		teken_subr_single_height_single_width_line(t);
		break;
	case '6': /* DECDWL: Single Height Double Width Line */
		teken_subr_single_height_double_width_line(t);
		break;
	case '8': /* DECALN: Alignment test */
		teken_subr_alignment_test(t);
		break;
	default:
		teken_printf("Unsupported sequence in teken_state_4: %u\n", (unsigned int)c);
		break;
	}

	teken_state_switch(t, teken_state_init);
}
