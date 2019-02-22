#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

PSP_MODULE_INFO("2048", 0, 1, 1);

PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define printf	pspDebugScreenPrintf

void dump_threadstatus(void);

int done = 0;

// 以下代码与本程序的退出有关

int exit_callback(int arg1, int arg2, void *common)
{
	done = 1;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

// 以上代码与本程序的退出有关

// 以下是游戏逻辑

#define UP    1
#define DOWN  2
#define LEFT  3
#define RIGHT 4

#define uint unsigned int

static uint mat[4][4];
static uint score = 0;

// 悔棋记忆
static uint PrevMat[16][16];
static uint PrevScore[16];
static uint BkUpCnt = 0;


// 返回某一元素上/下/左/右的值
int getNext(int row, int col, int dir)
{
	int i = 0;
	switch(dir){
		case UP:
			
			for(i = row - 1; i >= 0; i--)
			{
				if(mat[i][col] != 0)
					return mat[i][col];
			}
			return 0;
		case DOWN:
			for(i = row + 1; i <= 3; i++)
			{
				if(mat[i][col] != 0)
					return mat[i][col];
			}
			return 0;
		case LEFT:
			for(i = col - 1; i >= 0; i--)
			{
				if(mat[row][i] != 0)
					return mat[row][i];
			}
			return 0;
		case RIGHT:
			for(i = col + 1; i <= 3; i++)
			{
				if(mat[row][i] != 0)
					return mat[row][i];
			}
			return 0;
		default:
			return 0;
	}
}


// 返回某一元素上/下/左/右最远空位的下标
int getFarVoidIndex(int row, int col, int dir)
{
	int i = 0;
	switch(dir){
		case UP:
			for(i = row; i > 0; i--)
			{
				if((mat[i][col] == 0 || mat[row][col]) && mat[i-1][col] != 0)
					return i;
			}
			return 0;
		case DOWN:
			for(i = row; i < 3; i++)
			{
				if((mat[i][col] == 0 || mat[row][col]) && mat[i+1][col] != 0)
					return i;
			}
			return 3;
		case LEFT:
			for(i = col; i > 0; i--)
			{
				if((mat[row][i] == 0 || mat[row][col]) && mat[row][i-1] != 0)
					return i;
			}
			return 0;
		case RIGHT:
			for(i = col; i < 3; i++)
			{
				if((mat[row][i] == 0 || mat[row][col]) && mat[row][i+1] != 0)
					return i;
			}
			return 3;
		default:
			return 0;
	}
}


// 移动并合并
int Merge(int dir)
{
	int col = 0;
	int row = 0;
	switch(dir){
		case UP:
			for(col = 0; col < 4; col++)
			{
				for(row = 0; row < 3; row++)
				{
					if(mat[row][col] != 0 && mat[row][col] == getNext(row, col, DOWN))
					{
						// 合并
						mat[row][col] = mat[row][col] * 2;
						score += mat[row][col];
						// 邻位清零
						int neighbourIndex = getFarVoidIndex(row, col, DOWN) + 1;
						mat[neighbourIndex][col] = 0;
					}
				}
				for(row = 1; row < 4; row++)
				{
					if(mat[row][col] != 0)
					{
						int i = getFarVoidIndex(row, col, UP);
						if(i != row)
						{
							mat[i][col] = mat[row][col];
							mat[row][col] = 0;
						}
					}
				}
			}
			break;
		case DOWN:
			for(col = 0; col < 4; col++)
			{
				for(row = 3; row > 0; row--)
				{
					if(mat[row][col] != 0 && mat[row][col] == getNext(row, col, UP))
					{
						// 合并
						mat[row][col] = mat[row][col] * 2;
						score += mat[row][col];
						// 邻位清零
						int neighbourIndex = getFarVoidIndex(row, col, UP) - 1;
						mat[neighbourIndex][col] = 0;
					}
				}
				for(row = 2; row >= 0; row--)
				{
					if(mat[row][col] != 0)
					{
						int i = getFarVoidIndex(row, col, DOWN);
						if(i != row)
						{
							mat[i][col] = mat[row][col];
							mat[row][col] = 0;
						}
					}
				}
			}
			break;
		case LEFT:
			for(row = 0; row < 4; row++)
			{
				for(col = 0; col < 3; col++)
				{
					if(mat[row][col] != 0 && mat[row][col] == getNext(row, col, RIGHT))
					{
						mat[row][col] = mat[row][col] * 2;
						score += mat[row][col];
						// 邻位清零
						int neighbourIndex = getFarVoidIndex(row, col, RIGHT) + 1;
						mat[row][neighbourIndex] = 0;
					}
				}
				for(col = 1; col < 4; col++)
				{
					if(mat[row][col] != 0)
					{
						int i = getFarVoidIndex(row, col, LEFT);
						if(i != col)
						{
							mat[row][i] = mat[row][col];
							mat[row][col] = 0;
						}
					}
				}
			}
			break;
		case RIGHT:
			for(row = 0; row < 4; row++)
			{
				for(col = 3; col > 0; col--)
				{
					if(mat[row][col] != 0 && mat[row][col] == getNext(row, col, LEFT))
					{
						mat[row][col] = mat[row][col] * 2;
						score += mat[row][col];
						// 邻位清零
						int neighbourIndex = getFarVoidIndex(row, col, LEFT) - 1;
						mat[row][neighbourIndex] = 0;
					}
				}
				for(col = 2; col >= 0; col--)
				{
					if(mat[row][col] != 0)
					{
						int i = getFarVoidIndex(row, col, RIGHT);
						if(i != col)
						{
							mat[row][i] = mat[row][col];
							mat[row][col] = 0;
						}
					}
				}
			}
			break;
		default:
			break;
	}
	return 0;
}

// 返回游戏状态
int gameStatus()
{
	int i = 0;
	// 1  代表尚有可合并
	// 0  代表全填满且不可合并
	// -1 代表未填满
	for(i = 0; i < 16; i++)
	{
		if( !mat[i/4][i%4] )
			return -1;
	}
	for(i = 0; i < 16; i++)
	{
		if( getNext(i/4, i%4, UP   ) == mat[i/4][i%4] ||
			getNext(i/4, i%4, DOWN ) == mat[i/4][i%4] ||
			getNext(i/4, i%4, LEFT ) == mat[i/4][i%4] ||
			getNext(i/4, i%4, RIGHT) == mat[i/4][i%4] )
		{
			return 1;
		}
	}
	return 0;
}


// 方向验证
int DirVerify(int dir)
{
	int i = 0;
	switch(dir)
	{
	case UP:
		for(i = 4; i < 16; i++)
		{
			if( mat[i/4][i%4] != 0 && ( mat[i/4-1][i%4] == mat[i/4][i%4] || mat[i/4-1][i%4] == 0 ) )
			{
				return 1;
			}
		}
		return 0;
	case DOWN:
		for(i = 0; i < 12; i++)
		{
			if( mat[i/4][i%4] != 0 && ( mat[i/4+1][i%4] == mat[i/4][i%4] || mat[i/4+1][i%4] == 0 ) )
			{
				return 1;
			}
		}
		return 0;
	case LEFT:
		for(i = 4; i < 16; i++)
		{
			if( mat[i%4][i/4] != 0 && ( mat[i%4][i/4-1] == mat[i%4][i/4] || mat[i%4][i/4-1] == 0 ) )
			{
				return 1;
			}
		}
		return 0;
	case RIGHT:
		for(i = 0; i < 12; i++)
		{
			if( mat[i%4][i/4] != 0 && ( mat[i%4][i/4+1] == mat[i%4][i/4] || mat[i%4][i/4+1] == 0 ) )
			{
				return 1;
			}
		}
		return 0;
	default:
		return 0;
	}
}

// 随机插入
int ranInsert()
{
	int i = 0;
	int voidCount = 0;
	int voidIndex[16];
	if(gameStatus() == -1){
		for(i = 0; i < 16; i++)
		{
			if(!mat[i / 4][i % 4])
			{
				voidIndex[voidCount] = i;
				voidCount++;
			}
		}
		srand(time(0));
		int ranOffset = voidIndex[ rand() % voidCount ];
		mat[ranOffset / 4][ranOffset % 4] = 2;
	}
	return 0;
}

// 初始化
void init()
{
	int c = 0;
	for(c = 0; c < 16; c++){
		mat[c/4][c%4] = 0;
	}
	ranInsert();
	ranInsert();
}

// 记录游戏状态
void BackUp()
{
	int i = 0;
	int j = 0;
	
	for(i = 15; i >= 1; i--)
	{
		for(j = 0; j < 16; j++)
		{
			PrevMat[i][j] = PrevMat[i-1][j];
		}
		PrevScore[i] = PrevScore[i-1];
	}
	
	for(j = 0; j < 16; j++)
	{
		PrevMat[0][j] = mat[j/4][j%4];
		PrevScore[0] = score;
	}
}

// 读取游戏状态
void BackUpOut(int line)
{
	int j = 0;
	
	for(j = 0; j < 16; j++)
	{
		mat[j/4][j%4] = PrevMat[line][j];
		score = PrevScore[line];
	}
}

// 执行移动操作
int action(int dir)
{
	if(DirVerify(dir) != 0)
	{
		Merge(dir);
		
		BackUp();
		pspDebugScreenSetXY(0, 17);
		pspDebugScreenSetTextColor(0xffffffff);
		printf(" Backup matrix %d saved\n", BkUpCnt+1);
		if(BkUpCnt < 15)
		{
			BkUpCnt++;
		}

		ranInsert();
	}
	else
	{
		return 0;
	}
	
	return 0;
}

// 输出
void print(int score)
{
	int i = 0;
	int j = 0;

	pspDebugScreenSetXY(5, 3);
	printf("SCORE: %d", score);

	pspDebugScreenSetXY(5, 5);
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(!mat[i][j])
			{
				pspDebugScreenSetTextColor(0xffffffff);
				pspDebugScreenSetXY(5+6*j, 5+2*i);
				printf("..    ");
			}
			else
			{
				pspDebugScreenSetXY(5+6*j, 5+2*i);
				if(mat[i][j] == 2)
				{
					// 青色
					pspDebugScreenSetTextColor(0xffff00ff);
				}
				else if(mat[i][j] == 4)
				{
					// 蓝色
					pspDebugScreenSetTextColor(0xffff0000);
				}
				else if(mat[i][j] == 8)
				{
					// 水绿色
					pspDebugScreenSetTextColor(0xff7fff00);
				}
				else if(mat[i][j] == 16)
				{
					// 绿色
					pspDebugScreenSetTextColor(0xff00ff00);
				}
				else if(mat[i][j] == 32)
				{
					// 黄色
					pspDebugScreenSetTextColor(0xff00ffff);
				}
				else if(mat[i][j] == 64)
				{
					// 橙色
					pspDebugScreenSetTextColor(0xff407fff);
				}
				else if(mat[i][j] == 128)
				{
					// 红色
					pspDebugScreenSetTextColor(0xff0000ff);
				}
				else if(mat[i][j] == 256)
				{
					//粉红色
					pspDebugScreenSetTextColor(0xff7f40ff);
				}
				else if(mat[i][j] == 512)
				{
					// 紫色
					pspDebugScreenSetTextColor(0xffff00ff);
				}
				else if(mat[i][j] == 1024)
				{
					pspDebugScreenSetTextColor(0xffff007f);
				}
				else if(mat[i][j] == 2048)
				{
					pspDebugScreenSetTextColor(0xffffffff);
				}
				printf("%-6d", mat[i][j]);
			}
		}
		printf("\n");
	}
	printf("\n");
	pspDebugScreenSetTextColor(0xffffffff);
}



int main(void)
{
	int time = 0;
	
	SceCtrlData pad;

	pspDebugScreenInit();
	SetupCallbacks();
	
	printf("== 2048 on PlayStationPortable ==\n");
	printf("   by Mikukonai     2015.08.05   \n\n");

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

	init();
	print(score);

	while(!done)
	{
		sceCtrlReadBufferPositive(&pad, 1); 
		if (pad.Buttons != 0 && pad.TimeStamp - time > 300000)
		{
			time = pad.TimeStamp;
			pspDebugScreenSetXY(0, 19);
			pspDebugScreenSetTextColor(0xffffffff);
			//printf(" Key TimeStamp = %d us", time);
			
			if (pad.Buttons & PSP_CTRL_UP/* || pad.Buttons & PSP_CTRL_TRIANGLE*/)
			{
				action(UP);
				print(score);
				if(!gameStatus())
				{
					pspDebugScreenSetXY(5, 13);
					pspDebugScreenSetTextColor(0xff0000ff);
					printf("Game Over! >_<\n");
				}
			} 
			if (pad.Buttons & PSP_CTRL_DOWN/* || pad.Buttons & PSP_CTRL_CROSS*/)
			{
				action(DOWN);
				print(score);
				if(!gameStatus())
				{
					pspDebugScreenSetXY(5, 13);
					pspDebugScreenSetTextColor(0xff0000ff);
					printf("Game Over! >_<\n");
				}
			} 
			if (pad.Buttons & PSP_CTRL_LEFT /*|| pad.Buttons & PSP_CTRL_SQUARE*/)
			{
				action(LEFT);
				print(score);
				if(!gameStatus())
				{
					pspDebugScreenSetXY(5, 13);
					pspDebugScreenSetTextColor(0xff0000ff);
					printf("Game Over! >_<\n");
				}
			} 
			if (pad.Buttons & PSP_CTRL_RIGHT /*|| pad.Buttons & PSP_CTRL_CIRCLE*/)
			{
				action(RIGHT);
				print(score);
				if(!gameStatus())
				{
					pspDebugScreenSetXY(5, 13);
					pspDebugScreenSetTextColor(0xff0000ff);
					printf("Game Over! >_<\n");
				}
			}      

			if (pad.Buttons & PSP_CTRL_SQUARE)
			{
				if(BkUpCnt >= 0)
				{
					BackUpOut(15 - BkUpCnt);
					print(score);
					pspDebugScreenSetXY(0, 17);
					pspDebugScreenSetTextColor(0xffffffff);
					printf(" Backup matrix %d loaded\n", BkUpCnt+1);
					BkUpCnt--;
				}
				else if(BkUpCnt < 0)
				{
					pspDebugScreenSetXY(0, 17);
					pspDebugScreenSetTextColor(0xffffffff);
					printf(" Cannot load backup matrix...\n");
				}
			}
		}
	}

	sceKernelExitGame();
	return 0;
}
