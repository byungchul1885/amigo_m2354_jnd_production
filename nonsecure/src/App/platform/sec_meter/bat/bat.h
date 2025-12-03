#ifndef BAT_H
#define BAT_H 1


#define	NOBAT_THRSHLD_DEF			2.7
#define VBAT_ERR_LEVEL						1.21
#define	NOBAT_LEVEL_THRSHLD		NOBAT_THRSHLD_DEF
#define	NOBAT_LEVEL_PRD					3
#define	NOBAT_DECISION_CNT			5

typedef enum {
	BATSTATE_NOT_FIXED, BATSTATE_IN, BATSTATE_OUT
} bat_state_type;

extern bat_state_type bat_state;
extern bool bat_test_ready;
extern int nobat_cnt;

void bat_meas(void);
float get_bat_level(void);



#endif // BAT_H
