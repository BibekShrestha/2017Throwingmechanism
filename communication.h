/*
 * communication.h
 *
 * Created: 5/8/2017 5:03:08 PM
 *  Author: Rajesh
 */ 


#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_



#define DECREASE_SECOND_MOTOR_SPEED_BIT		7
#define INCREASE_SECOND_MOTOR_SPEED_BIT		6
#define DECREASE_FIRST_MOTOR_SPEED_BIT		5
#define INCREASE_FIRST_MOTOR_SPEED_BIT		4
#define	STOP_PLATFORM_MOTOR_BIT		 		3
#define	START_PLATFORM_MOTOR_BIT	 		2
#define DATA_RQ_BIT							1
#define VALIDITY_BIT						0



#define STATUS_DE_VALIDITY_BIT( Rx_Buffer )						(Rx_Buffer & ( 1 << VALIDITY_BIT					))	>>VALIDITY_BIT
#define STATUS_DE_DATA_RQ_BIT( Rx_Buffer )						(Rx_Buffer & ( 1 << DATA_RQ_BIT						))	>>DATA_RQ_BIT
#define STATUS_DE_START_PLATFORM_MOTOR_BIT( Rx_Buffer ) 		(Rx_Buffer & ( 1 << START_PLATFORM_MOTOR_BIT	 	))	>>START_PLATFORM_MOTOR_BIT
#define STATUS_DE_STOP_PLATFORM_MOTOR_BIT( Rx_Buffer )		 	(Rx_Buffer & ( 1 << STOP_PLATFORM_MOTOR_BIT		 	))	>>STOP_PLATFORM_MOTOR_BIT
#define STATUS_DE_INCREASE_FIRST_MOTOR_SPEED_BIT( Rx_Buffer )	(Rx_Buffer & ( 1 << INCREASE_FIRST_MOTOR_SPEED_BIT	))	>>INCREASE_FIRST_MOTOR_SPEED_BIT
#define STATUS_DE_DECREASE_FIRST_MOTOR_SPEED_BIT( Rx_Buffer )	(Rx_Buffer & ( 1 << DECREASE_FIRST_MOTOR_SPEED_BIT	))	>>DECREASE_FIRST_MOTOR_SPEED_BIT
#define STATUS_DE_INCREASE_SECOND_MOTOR_SPEED_BIT( Rx_Buffer )	(Rx_Buffer & ( 1 << INCREASE_SECOND_MOTOR_SPEED_BIT ))	>>INCREASE_SECOND_MOTOR_SPEED_BIT
#define STATUS_DE_DECREASE_SECOND_MOTOR_SPEED_BIT( Rx_Buffer )	(Rx_Buffer & ( 1 << DECREASE_SECOND_MOTOR_SPEED_BIT ))	>>DECREASE_SECOND_MOTOR_SPEED_BIT


#define RESET_VALIDITY_BIT( Rx_Buffer )							Rx_Buffer &= ~( 1 << VALIDITY_BIT					)
#define RESET_DATA_RQ_BIT( Rx_Buffer )							Rx_Buffer &= ~( 1 << DATA_RQ_BIT					)
#define RESET_START_PLATFORM_MOTOR_BIT( Rx_Buffer ) 			Rx_Buffer &= ~( 1 << START_PLATFORM_MOTOR_BIT	 	)
#define RESET_STOP_PLATFORM_MOTOR_BIT( Rx_Buffer )		 		Rx_Buffer &= ~( 1 << STOP_PLATFORM_MOTOR_BIT		)
#define RESET_INCREASE_FIRST_MOTOR_SPEED_BIT( Rx_Buffer )		Rx_Buffer &= ~( 1 << INCREASE_FIRST_MOTOR_SPEED_BIT	)
#define RESET_DECREASE_FIRST_MOTOR_SPEED_BIT( Rx_Buffer )		Rx_Buffer &= ~( 1 << DECREASE_FIRST_MOTOR_SPEED_BIT	)
#define RESET_INCREASE_SECOND_MOTOR_SPEED_BIT( Rx_Buffer )		Rx_Buffer &= ~( 1 << INCREASE_SECOND_MOTOR_SPEED_BIT)
#define RESET_DECREASE_SECOND_MOTOR_SPEED_BIT( Rx_Buffer )		Rx_Buffer &= ~( 1 << DECREASE_SECOND_MOTOR_SPEED_BIT)




#define SET_VALIDITY_BIT( Rx_Buffer )							Rx_Buffer |= (1	<<	VALIDITY_BIT					)
#define SET_DATA_RQ_BIT( Rx_Buffer )							Rx_Buffer |= (1	<<	DATA_RQ_BIT						)
#define SET_START_PLATFORM_MOTOR_BIT( Rx_Buffer )	 			Rx_Buffer |= (1	<<	START_PLATFORM_MOTOR_BIT	 	);\
																					RESET_STOP_PLATFORM_MOTOR_BIT(Rx_Buffer)	 ;\
																					SET_VALIDITY_BIT(Rx_Buffer)
#define SET_STOP_PLATFORM_MOTOR_BIT( Rx_Buffer )		 		Rx_Buffer |= (1	<<	STOP_PLATFORM_MOTOR_BIT		 	);\
																					RESET_START_PLATFORM_MOTOR_BIT	;\
																					SET_VALIDITY_BIT(Rx_Buffer)
#define SET_INCREASE_FIRST_MOTOR_SPEED_BIT( Rx_Buffer )			Rx_Buffer |= (1	<<	INCREASE_FIRST_MOTOR_SPEED_BIT	);\
																					SET_VALIDITY_BIT(Rx_Buffer)
#define SET_DECREASE_FIRST_MOTOR_SPEED_BIT( Rx_Buffer )			Rx_Buffer |= (1	<<	DECREASE_FIRST_MOTOR_SPEED_BIT	);\
																					SET_VALIDITY_BIT(Rx_Buffer)
#define SET_INCREASE_SECOND_MOTOR_SPEED_BIT( Rx_Buffer )		Rx_Buffer |= (1	<<	INCREASE_SECOND_MOTOR_SPEED_BIT	);\
																					SET_VALIDITY_BIT(Rx_Buffer)
#define SET_DECREASE_SECOND_MOTOR_SPEED_BIT( Rx_Buffer )		Rx_Buffer |= (1	<<	DECREASE_SECOND_MOTOR_SPEED_BIT	);\
																					SET_VALIDITY_BIT(Rx_Buffer)





#endif /* COMMUNICATION_H_ */