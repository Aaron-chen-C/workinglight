#include "app_config.h"
#include "lamp_app.h"

static uint16_t key1_press_time = 0;
static uint16_t key2_press_time = 0;
static uint16_t key3_press_time = 0;
static uint8_t key1_long_press_active = 0;
static uint8_t key3_long_press_active = 0;
static uint8_t key1_last_direction = 0; // 0: decrease, 1: increase
static uint8_t key3_last_direction = 0; // 0: decrease, 1: increase
static uint16_t key1_last_dim_time = 0;
static uint16_t key3_last_dim_time = 0;
static uint8_t key1_pressed_flag = 0; // 0: not pressed, 1: pressed
static uint8_t key2_pressed_flag = 0;
static uint8_t key3_pressed_flag = 0;

extern lamp_para_t deviceParam;

/**
 * @brief 亮度曲线转换函数
 * @param lightness 输入亮度值 (0-10000)
 * @return uint16_t 转换后的亮度值
 */
uint16_t lightness_to_curve(uint16_t lightness)
{
	uint16_t lightness_curve;

	if (lightness <= 1000)
	{
		lightness_curve = (uint16_t)(0.53 * lightness);
	}
	else if (lightness > 1000 && lightness <= 2000)
	{
		lightness_curve = 535 + (lightness - 1000) * 0.55; // 1085
	}
	else if (lightness > 2000 && lightness <= 3000)
	{
		lightness_curve = 1148 + (lightness - 2000) * 0.63; // 1778
	}
	else if (lightness > 3000 && lightness <= 4000)
	{
		lightness_curve = 1846 + (lightness - 3000) * 0.71; // 2556
	}
	else if (lightness > 4000 && lightness <= 5000)
	{
		lightness_curve = 2639 + (lightness - 4000) * 0.81; // 3449
	}
	else if (lightness > 5000 && lightness <= 6000)
	{
		lightness_curve = 3535 + (lightness - 5000) * 0.91; // 4445
	}
	else if (lightness > 6000 && lightness <= 7000)
	{
		lightness_curve = 4547 + (lightness - 6000) * 1.02; // 5567
	}
	else if (lightness > 7000 && lightness <= 8000)
	{
		lightness_curve = 5685 + (lightness - 7000) * 1.15; // 6835
	}
	else if (lightness > 8000 && lightness <= 9000)
	{
		lightness_curve = 6964 + (lightness - 8000) * 1.29; // 8254
	}
	else if (lightness > 9000 && lightness < 10000)
	{
		lightness_curve = 8343 + (lightness - 9000) * 1.6; // 8194
	}
	if (lightness == MAX_BRIGHTNESS_VALUE)
	{
		lightness_curve = MAX_BRIGHTNESS_VALUE;
	}

	return lightness_curve;
}

void ADC_CaptureProcess(void)
{
	uint16_t ADC_value4 = 0;
	uint16_t ADC_value5 = 0;
	uint16_t flag1 = 0;
	uint16_t flag2 = 0;
	static uint16_t timer_1ms = 0;
	static uint8_t initflag=1;
	uint16_t loguplightness = 0;
	uint16_t logdownlightness = 0;
	static uint16_t last_up_bright = 0;
	static uint16_t last_down_bright = 0;

	

	timer_1ms++;
	if (timer_1ms >= 10) // 10ms timer for key scanning
	{
		timer_1ms = 0;
		lighting_hsl_server_t *hsl_pdata = light_server_data_get();
		if (ADC->DR4_b.DONE == 1)
		{
			ADC_value4 = (ADC->DR4_b.RESULT & 0x0fff); // 
//			if (ADC_value4 < 2048 )  //low  power
//			{
//				flag1 = 0;
//			}
		
			if (ADC_value4 > 2048)
			{
				flag1 = 1;
				key1_press_time++;
				key1_pressed_flag = 1;
			}
			if (ADC_value4 < 2048 )  //low  power
			{
				flag1 = 0;
				// Key 1 release - short press detection
				if (key1_pressed_flag == 1 && key1_press_time < LONG_TIME_PRESS && key1_press_time >= SHORT_TIME_PRESS  )
				{
					
//						light_transition_func(UPP, 0, 500, ST_TRANS_RGB_R);
//           light_transition_func(hsl_pdata->preset_rgb_g, 0, 500, ST_TRANS_RGB_G);
					// Short press: control both lights
					if (KEY1SWITCH == 1 )
					{
						KEY1SWITCH = 0;
						light_transition_func(UPP, 0, 100, ST_TRANS_RGB_R);
					}
					else 
					{
						KEY1SWITCH = 1;
						loguplightness = lightness_to_curve(UPBRIGHTNESS);
						hsl_pdata->target_rgb_r = loguplightness;
						light_transition_func(UPP, UPT, 500, ST_TRANS_RGB_R);
					}
//					else
//					{
//						// One on one off, turn both off
//						KEY2SWITCH = 0;
//						KEY1SWITCH = 0;
//						light_transition_func(hsl_pdata->preset_rgb_g, 0, 250, ST_TRANS_RGB_G);
//						light_transition_func(hsl_pdata->preset_rgb_r, 0, 250, ST_TRANS_RGB_R);
//					}
					if (key3_pressed_flag == 1 || key1_pressed_flag == 1)
					{
					// Log_print("switch1:%d  switch2:%d\r\n",  KEY1SWITCH, KEY2SWITCH);
//					Log_print("save success\r\n");
						save_device_para_all1(); //up&down/brightness&direction  save
					}

				
//				key3_pressed_flag = 0;
				
				}
				key1_press_time = 0;
//				key3_press_time = 0;
				key1_long_press_active = 0;
//				key3_long_press_active = 0;
				key1_pressed_flag = 0;
				// Reset all key states
//				key3_press_time = 0;
			//	key1_long_press_active = 0;
//				key3_pressed_flag = 0;
			}
			// Key 1 processing - long press
			if (flag1 == 1)
			{
				// Long press detection (10ms * 150 = 1500ms)
				if (key1_press_time == LONG_TIME_PRESS && key1_long_press_active == 0 && KEY1SWITCH == 1)
				{
					key1_long_press_active = 1;
					key1_last_dim_time = 0;

					// Determine initial direction
					if (UPBRIGHTNESS >= MAXLIGHTNESS) // Max brightness
					{
						UP_DIRECTION = 0; // Decrease
					}
					else if (UPBRIGHTNESS <= MINLIGHTNESS) // Min brightness
					{
						UP_DIRECTION = 1; // Increase
					}
					// Else keep last direction
				//		key1_press_time = 0;
				}

				// Long press dimming (every 10ms * 10 = 100ms)
				if (key1_long_press_active == 1 && KEY1SWITCH == 1)
				{
					key1_last_dim_time++;
					if (key1_last_dim_time >= ADJUST_TIME) // 100ms interval
					{
						key1_last_dim_time = 0;

						// Apply dimming based on direction
						if (UP_DIRECTION == 1) // Increase
						{
							if (UPBRIGHTNESS < MAXLIGHTNESS)
							{
								UPBRIGHTNESS += STEP_VALUE;
								if (UPBRIGHTNESS > MAXLIGHTNESS)
									UPBRIGHTNESS = MAXLIGHTNESS;
							}
						}
						else // Decrease
						{
							if (UPBRIGHTNESS > MINLIGHTNESS)
							{
								UPBRIGHTNESS -= STEP_VALUE;
								if (UPBRIGHTNESS < MINLIGHTNESS)
									UPBRIGHTNESS = MINLIGHTNESS;
							}
						}
						loguplightness = lightness_to_curve(UPBRIGHTNESS);
						hsl_pdata->target_rgb_r = loguplightness;
						light_transition_func(UPP, UPT, 500, ST_TRANS_RGB_R);
						save_device_para_all1();
					}
				}
			}
		}
		if (ADC->DR5_b.DONE == 1)
		{
			ADC_value5 = (ADC->DR5_b.RESULT & 0x0fff); // 
//			if (ADC_value4 < 2048 )  //low  power
//			{
//				flag1 = 0;
//			}
		
			if (ADC_value5 > 2048)
			{
				flag2 = 1;
				key3_press_time++;
				key3_pressed_flag = 1;
			}
			if (ADC_value5 < 2048 )  //low  power
			{
				flag2 = 0;
				// Key 1 release - short press detection
				if (key3_pressed_flag == 1 && key3_press_time < LONG_TIME_PRESS && key3_press_time >= SHORT_TIME_PRESS  )
				{
					
//						light_transition_func(UPP, 0, 500, ST_TRANS_RGB_R);
//           light_transition_func(hsl_pdata->preset_rgb_g, 0, 500, ST_TRANS_RGB_G);
					// Short press: control both lights
					if (KEY2SWITCH == 1 )
					{
						KEY2SWITCH = 0;
						light_transition_func(DOWNP, 0, 100, ST_TRANS_RGB_G);
					}
					else 
					{
						KEY2SWITCH = 1;
						logdownlightness = lightness_to_curve(DOWNBRIGHTNESS);
						hsl_pdata->target_rgb_g = logdownlightness;
						light_transition_func(DOWNP, DOWNT, 500, ST_TRANS_RGB_G);
					}
//					else
//					{
//						// One on one off, turn both off
//						KEY2SWITCH = 0;
//						KEY1SWITCH = 0;
//						light_transition_func(hsl_pdata->preset_rgb_g, 0, 250, ST_TRANS_RGB_G);
//						light_transition_func(hsl_pdata->preset_rgb_r, 0, 250, ST_TRANS_RGB_R);
//					}


				
//				key3_pressed_flag = 0;
				
				}
				key3_press_time = 0;
//				key3_press_time = 0;
				key3_long_press_active = 0;
//				key3_long_press_active = 0;
				key3_pressed_flag = 0;
				// Reset all key states
//				key3_press_time = 0;
			//	key1_long_press_active = 0;
//				key3_pressed_flag = 0;
			
			}
			// Key 1 processing - long press
			if (flag2 == 1)
			{
				// Long press detection (10ms * 150 = 1500ms)
				if (key3_press_time == LONG_TIME_PRESS && key3_long_press_active == 0 && KEY2SWITCH == 1)
				{
					key3_long_press_active = 1;
					key3_last_dim_time = 0;

					// Determine initial direction
					if (DOWNBRIGHTNESS >= MAXLIGHTNESS) // Max brightness
					{
						DOWN_DIRECTION = 0; // Decrease
					}
					else if (DOWNBRIGHTNESS <= MINLIGHTNESS) // Min brightness
					{
						DOWN_DIRECTION = 1; // Increase
					}
					// Else keep last direction
				//		key1_press_time = 0;
				}

				// Long press dimming (every 10ms * 10 = 100ms)
				if (key3_long_press_active == 1 && KEY2SWITCH == 1)
				{
					key3_last_dim_time++;
					if (key3_last_dim_time >= ADJUST_TIME) // 100ms interval
					{
						key3_last_dim_time = 0;

						// Apply dimming based on direction
						if (DOWN_DIRECTION == 1) // Increase
						{
							if (DOWNBRIGHTNESS < MAXLIGHTNESS)
							{
								DOWNBRIGHTNESS += STEP_VALUE;
								if (DOWNBRIGHTNESS > MAXLIGHTNESS)
									DOWNBRIGHTNESS = MAXLIGHTNESS;
							}
						}
						else // Decrease
						{
							if (DOWNBRIGHTNESS > MINLIGHTNESS)
							{
								DOWNBRIGHTNESS -= STEP_VALUE;
								if (DOWNBRIGHTNESS < MINLIGHTNESS)
									DOWNBRIGHTNESS = MINLIGHTNESS;
							}
						}
						logdownlightness = lightness_to_curve(DOWNBRIGHTNESS);
						hsl_pdata->target_rgb_g = logdownlightness;
						light_transition_func(DOWNP, DOWNT, 500, ST_TRANS_RGB_G);
						save_device_para_all1(); //up&down/brightness&direction  save
					}
				}
			}
			
			if (key1_press_time >= 200)
			{
				key1_press_time = 160;
			}
			if (key3_press_time >= 200)
			{
				key3_press_time = 160;
			}
			
	   }
		START_AD_CONVERTION();
		}
	}
 

//	if (ADC->DR5_b.DONE == 1)
//	  {
//			 ADC_value5 = (ADC->DR5_b.RESULT & 0x0fff); // 
//			 if (ADC_value5 < 2048 )  //low  power
//			{
//				flag2 = 0;
//			}
//			if (ADC_value5 > 2048)
//			{
//				flag2 = 1;
//				key3_press_time++;
////				key1_press_time = 0;
//				key3_pressed_flag = 1;
//				// Key 1 release - short press detection
//				if (key3_pressed_flag == 1 && key3_press_time >= SHORT_TIME_PRESS && key3_press_time < LONG_TIME_PRESS)
//				{
//					// Short press: control both lights
//					if (KEY2SWITCH == 1)
//					{
//						// Both lights on, turn both off
//						KEY2SWITCH = 0;
//					light_transition_func(hsl_pdata->preset_rgb_g, 0, 250, ST_TRANS_RGB_G);
////						light_transition_func(hsl_pdata->preset_rgb_r, 0, 250, ST_TRANS_RGB_R);
//					}
//					else if (KEY2SWITCH == 0)
//					{
//						// Both lights off, turn both on
//						KEY2SWITCH = 1;
//						light_transition_func(DOWNP, DOWNT, 250, ST_TRANS_RGB_G);// 当前值
////						light_transition_func(UPP, UPT, 250, ST_TRANS_RGB_R);
//					}			
//					save_device_para_all1();
//				}
//				// Reset all key states
////				key1_press_time = 0;
//				key3_long_press_active = 0;
////				key1_pressed_flag = 0;
//			}

//			// Key 1 processing - long press
//			if (flag2 == 1)
//			{
//				// Long press detection (10ms * 150 = 1500ms)
//					if (key3_press_time == LONG_TIME_PRESS && key3_long_press_active == 0 && KEY2SWITCH == 1)
//				{
//					key3_long_press_active = 1;
//					key3_last_dim_time = 0;

//					// Determine initial direction
//					if (DOWNBRIGHTNESS >= MAXLIGHTNESS) // Max brightness
//					{
//						DOWN_DIRECTION = 0; // Decrease
//					}
//					else if (DOWNBRIGHTNESS <= MINLIGHTNESS) // Min brightness
//					{
//						DOWN_DIRECTION = 1; // Increase
//					}
//					// Else keep last direction
//				}
//				// Long press dimming (every 10ms * 10 = 100ms)
//				if (key3_long_press_active == 1 && KEY2SWITCH == 1)
//				{
//					key3_last_dim_time++;
//					if (key3_last_dim_time >= ADJUST_TIME) // 100ms interval
//					{
//						key3_last_dim_time = 0;

//						// Apply dimming based on direction
//						if (DOWN_DIRECTION == 1) // Increase
//						{
//							if (DOWNBRIGHTNESS < MAXLIGHTNESS)
//							{
//								DOWNBRIGHTNESS += STEP_VALUE;
//								if (DOWNBRIGHTNESS > MAXLIGHTNESS)
//									DOWNBRIGHTNESS = MAXLIGHTNESS;
//							}
//						}
//						else // Decrease
//						{
//							if (DOWNBRIGHTNESS > MINLIGHTNESS)
//							{
//								DOWNBRIGHTNESS -= STEP_VALUE;
//								if (DOWNBRIGHTNESS < MINLIGHTNESS)
//									DOWNBRIGHTNESS = MINLIGHTNESS;
//							}
//						}
//						logdownlightness = lightness_to_curve(DOWNBRIGHTNESS);
//						hsl_pdata->target_rgb_g = logdownlightness;
//						light_transition_func(DOWNP, DOWNT, 250, ST_TRANS_RGB_G);
//					}
//				}
//			}
////			if (key1_press_time >= 200)
////			{
////				key1_press_time = 160;
////			}

//			if (key3_press_time >= 200)
//			{
//				key3_press_time = 160;
//			}
////			if (key2_press_time >= 200)
////			{
////				key2_press_time = 160;
////			}
//			if(initflag)
//			{
//					if (KEY1SWITCH == 0 && KEY2SWITCH == 0)
//			{
//				light_transition_func(hsl_pdata->preset_rgb_g, 0, 250, ST_TRANS_RGB_G);
//				light_transition_func(hsl_pdata->preset_rgb_r, 0, 250, ST_TRANS_RGB_R);
//				// save_device_para_all1();
//			}
//			if (KEY1SWITCH == 1 && KEY2SWITCH == 0)
//			{
//				light_transition_func(hsl_pdata->preset_rgb_g, 0, 250, ST_TRANS_RGB_G);
//				loguplightness = lightness_to_curve(UPBRIGHTNESS);
//				hsl_pdata->target_rgb_r = loguplightness;
//				light_transition_func(hsl_pdata->preset_rgb_r, hsl_pdata->target_rgb_r, 250, ST_TRANS_RGB_R);
//				//			light_transition_func(last_up_bright, loguplightness, 500, ST_TRANS_RGB_R);//PWM0
//				//			last_up_bright = loguplightness;
//			}
//			if (KEY1SWITCH == 0 && KEY2SWITCH == 1)
//			{
//				light_transition_func(hsl_pdata->preset_rgb_r, 0, 250, ST_TRANS_RGB_R);
//				logdownlightness = lightness_to_curve(DOWNBRIGHTNESS);
//				hsl_pdata->target_rgb_g = logdownlightness;
//				light_transition_func(hsl_pdata->preset_rgb_g, hsl_pdata->target_rgb_g, 250, ST_TRANS_RGB_G);
//				//			light_transition_func(last_down_bright, logdownlightness, 500, ST_TRANS_RGB_G);//PWM2
//				//			last_down_bright = logdownlightness;
//			}
//			if (KEY1SWITCH == 1 && KEY2SWITCH == 1)
//			{
//				logdownlightness = lightness_to_curve(DOWNBRIGHTNESS);
//				loguplightness = lightness_to_curve(UPBRIGHTNESS);
//				hsl_pdata->target_rgb_r = loguplightness;
//				hsl_pdata->target_rgb_g = logdownlightness;
//				light_transition_func(hsl_pdata->preset_rgb_r, hsl_pdata->target_rgb_r, 250, ST_TRANS_RGB_R);
//				light_transition_func(hsl_pdata->preset_rgb_g, hsl_pdata->target_rgb_g, 250, ST_TRANS_RGB_G);
//				//		Log_print("this way\r\n");
//				//			last_up_bright = loguplightness;
//				//			last_down_bright = logdownlightness;
//				// save_device_para_all1();
//			}
//		  initflag=0;
//	   }
//		}
		// Update lamp state if any key was pressed
//		START_AD_CONVERTION();
//	}
//}
