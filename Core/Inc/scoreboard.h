/*
 * scoreboard.h
 *
 *  Created on: Mar 18, 2024
 *      Author: josh
 */

#ifndef INC_SCOREBOARD_H_
#define INC_SCOREBOARD_H_

// Bit Definitions for the console_info
#define CONSOLE_SIGNATURE   (0b11000000) // Fixed signature bits
#define CONSOLE_IDENTIFIER  (0b00000111) // Between 1 and 5 (I2C addresses 0x10 - 0x14)
#define CONSOLE_CLOCK_SYNC  (0b00001000) // 0 = no, 1 = yes
#define CONSOLE_CLOCK_SHIFT (3)
#define GAME_LEVEL_MODE     (0b00110000) // 0 = easy, 1 = medium, 2 = hard, 3 = insane)
#define GAME_LEVEL_MODE_SHIFT    (4)


// Bit Definitions for the current_game_state
#define GAME_STATUS         (0b00000011) // 0 = stopped, 1 = running, 2 = paused, 3 = game over
#define GAME_STATUS_SHIFT   (0)
#define GAME_PLAYING_LEVEL  (0b11111100) // Between 1 and 63
#define GAME_PLAYING_LEVEL_SHIFT (2)

// Bit Definitions for the current_game_state2
#define GAME_NUM_PLAYERS  (0b00000001)  // 0 = one player, 1 = two player
#define GAME_POISON_FLAG  (0b00000010)  // 0 = no, 1 = yes
#define GAME_POISON_SHIFT (1)
#define GAME_SPEED        (0b11111100) // Between 0 and 60
#define GAME_SPEED_SHIFT  (2)

// 32-Bit Definitions for the date/time
// 31    26   22    17    12     6      0
// +------+----+-----+-----+------+------+
// |YYYYYY|MMMM|DDDDD|HHHHH|MMMMMM|SSSSSS|
// +------+----+-----+-----+------+------+
#define YEAR_MASK   (0b00111111 << 26)
#define YEAR_SHIFT  (26)
#define MONTH_MASK  (0b00001111 << 22)
#define MONTH_SHIFT (22)
#define DAY_MASK    (0b00011111 << 17)
#define DAY_SHIFT   (17)
#define HOUR_MASK   (0b00011111 << 12)
#define HOUR_SHIFT  (12)
#define MINUTE_MASK (0b00111111 << 6)
#define MINUTE_SHIFT (6)
#define SECOND_MASK (0b00111111)
#define SECOND_SHIFT (0)

typedef struct {                    // Register Map
    uint8_t console_info;           // 0x00
    uint8_t current_game_state;     // 0x01
    uint8_t current_game_state2;    // 0x02
    uint16_t current_score1;        // 0x03
    uint16_t current_score2;        // 0x05
    uint16_t number_apples1;        // 0x07
    uint16_t number_apples2;        // 0x09
    uint16_t high_score;            // 0x0B
    uint16_t playing_time;          // 0x0D
    uint16_t num_apples_easy;       // 0x0F
    uint16_t num_apples_medium;     // 0x11
    uint16_t num_apples_hard;       // 0x13
    uint16_t num_apples_insane;     // 0x15
    uint16_t high_score_easy;       // 0x17
    uint16_t high_score_medium;     // 0x19
    uint16_t high_score_hard;       // 0x1B
    uint16_t high_score_insane;     // 0x1D
    char initials_easy[3];          // 0x1F
    char initials_medium[3];        // 0x22
    char initials_hard[3];          // 0x25
    char initials_insane[3];        // 0x28
    uint32_t date_time;             // 0x2B
} scoreboard_t;

#endif /* INC_SCOREBOARD_H_ */
