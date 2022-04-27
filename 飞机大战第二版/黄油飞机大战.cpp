//若出现字符串报错，须在项目设置里将Unicode编码改为多字节字符编码
#include<stdio.h>
#include<easyx.h>
#include<graphics.h>
#include<stdlib.h>
#include<time.h>

//图形尺寸
#define windowwidth 768
#define windowheight 768
#define button1width 120
#define buttonheight 80


//游戏数据
#define playerplanewidth 40
#define playerplaneheight 60
float speedup = 0;//玩家初始向上速度为0
float speeddown = 0;
float speedleft = 0;
float speedright = 0;
#define accela 2//玩家加速度
#define accela2 0.5//无操作时玩家的减速度
#define maxspeed 10//玩家最大速度
#define bulletnum 20//同屏出现的玩家子弹数量
#define bulletinitialspeed 32//子弹初速度
#define bulletminspeed 5//子弹最小速度
#define bulletacc 0.8//子弹减速度
float bulletspeed[bulletnum];//每个子弹有单独的速度
#define bulletintervaltime 150//子弹发射间隔时间（单位：毫秒）
int score = 0;//游戏分数
int  lastingtime=0;//持续时间
clock_t start;//记录开始时间

#define enemynum 15
#define enemyplane0width 40
#define enemyplane0height 40
#define enemyplane1width 80
#define enemyplane1height 80
float enemyplanespeed[20];
float enemyspeedup = 0;//敌机总体加速度（也就是说敌机会随着游戏的进行变的越来越快）
#define enemymaxspeed 8
#define explosionradius0 40
#define explosionradius1 80
#define bulletexplosionradius 50

int close = 0;


//配色方案
#define navy BGR(0x000D6B)
#define purple BGR(0x9C19E0)
#define pink BGR(0xFF5DA2)
#define teal BGR(0x99DDCC)
//https://colorhunt.co/palette/000d6b9c19e0ff5da299ddcc


//定义贴图
IMAGE bkgimg1;//滚动背景1
IMAGE bkgimg2;//滚动背景2
IMAGE imgplayerplane0;//飞机掩码
IMAGE imgplayerplane1;//飞机源码
IMAGE imgbullet[2];
IMAGE imgenemyplane[2][2];

//定时器
bool timer1(int ms)
{
	static DWORD t1, t2;
	if (t2 - t1 > ms)
	{
		t1 = t2;
		return true;
	}
	t2 = clock();
	return false;
}
bool timer2(int ms)
{
	static DWORD t1, t2;
	if (t2 - t1 > ms)
	{
		t1 = t2;
		return true;
	}
	t2 = clock();
	return false;
}

//设置物体（包括玩家飞机、子弹、敌机等）
struct planeandbullet
{
	float x;
	float y;//坐标
	int isalive;//是否存活/存在
	int hp;//血量
	int width;//宽
	int height;//高
	int type;

	float speedx=0,speedy=0;
	int mass;
	//提供动量数据实现反弹

	int r;
}player,bullet[bulletnum],enemy[enemynum],explosion[enemynum],bulletexplosion[bulletnum];


//载入图像(提前载入部分图像，而非每帧渲染时重新载入，提高性能)
void loadimg()
{
	//滚动背景
	loadimage(&bkgimg1, "./pictures/background2.png",1024,769);//用1024x768好像衔接的时候会有一个像素的宽的白条，就改成769了
	loadimage(&bkgimg2, "./pictures/background2.png",1024,769);
	
	//玩家飞机
	loadimage(&imgplayerplane0, "./pictures/playerplane000.png", playerplanewidth, playerplaneheight);
	loadimage(&imgplayerplane1, "./pictures/playerplane111.png", playerplanewidth, playerplaneheight);
	
	//玩家子弹
	loadimage(&imgbullet[0], "./pictures/bullet00.png", 10, 20);
	loadimage(&imgbullet[1], "./pictures/bullet11.png", 10, 20);

	//敌机
	loadimage(&imgenemyplane[0][0], "./pictures/enemyplane0.png", enemyplane0width, enemyplane0height);
	loadimage(&imgenemyplane[0][1], "./pictures/enemyplane1.png", enemyplane0width, enemyplane0height);
	loadimage(&imgenemyplane[1][0], "./pictures/enemyplane00.png", enemyplane1width, enemyplane1height);
	loadimage(&imgenemyplane[1][1], "./pictures/enemyplane11.png", enemyplane1width, enemyplane1height);
}
void game();//声明

//定义可滚动的背景(滚动越来越快)
struct background
{
	float y;
}bkg1, bkg2;
//刷新静态游戏背景
void refreshbackground()
{
	IMAGE img1;
	loadimage(&img1, "./pictures/background2.png");
	putimage(0, 0, &img1);
}
//刷新滚动游戏背景
void refreshbackgroundscroll()
{
	if (bkg1.y < windowheight)bkg1.y+=0.5+enemyspeedup/40;
	else bkg1.y -= windowheight*2;
	putimage(0, bkg1.y, &bkgimg1);

	if (bkg2.y < windowheight)bkg2.y+=0.5+enemyspeedup/40;
	else bkg2.y -= windowheight*2;
	putimage(0, bkg2.y, &bkgimg2);
}



//生成一个圆角矩形按钮1（白底蓝字）
void button1(int x, int y, int width, int height, const char* text, int textsize)
{
	//设置按钮
	setfillcolor(WHITE);
	int btn1a = x - width / 2;
	int	btn1b = y - height / 2;
	int	btn1c = x + width / 2;
	int	btn1d = y + height / 2;
	solidroundrect(btn1a, btn1b, btn1c, btn1d, 5, 5);

	//设置文字
	setbkmode(TRANSPARENT);
	settextstyle(textsize, 0, "微软雅黑");//字体及大小
	settextcolor(navy);//颜色
	int length1 = textwidth(text) / 2;
	int height1 = textheight(text) / 2;
	outtextxy(x - length1, y - height1, text);
}
//生成一个圆角矩形按钮2（蓝底白字）
void button2(int x, int y, int width, int height, const char* text, int textsize)
{
	//设置按钮
	setfillcolor(navy);
	int btn1a = x - width / 2;
	int	btn1b = y - height / 2;
	int	btn1c = x + width / 2;
	int	btn1d = y + height / 2;
	solidroundrect(btn1a, btn1b, btn1c, btn1d, 5, 5);

	//设置文字
	setbkmode(TRANSPARENT);
	settextstyle(textsize, 0, "微软雅黑");//字体及大小
	settextcolor(WHITE);//颜色
	int length1 = textwidth(text) / 2;
	int height1 = textheight(text) / 2;
	outtextxy(x - length1, y - height1, text);
}
//游戏结束界面
void gameover()
{
	//设置文字
	int x = windowwidth / 2;
	int y = windowheight / 2;
	char text[] = "游戏结束";
	setbkmode(TRANSPARENT);
	settextstyle(100, 0, "微软雅黑");//字体及大小
	settextcolor(WHITE);//颜色
	int length1 = textwidth(text) / 2;
	int height1 = textheight(text) / 2;
	outtextxy(x - length1, y - height1-50, text);

	//设置按钮
	button1(windowwidth / 2-85, windowheight / 2+70, button1width+10, buttonheight-20, "重新开始",30);

	//设置按钮
	button1(windowwidth / 2+85, windowheight / 2+70, button1width+10, buttonheight-20, "退出游戏",30);

	//设置键鼠响应
	while (close==0)//一直接收消息
	{

        BeginBatchDraw();
		ExMessage msg1;//定义一个消息
		if (peekmessage(&msg1, EM_MOUSE))//如果有鼠标消息
		{
			switch (msg1.message)
			{
			    case WM_LBUTTONDOWN://左键按下时
				    if (msg1.x > ((windowwidth / 2 - 85 )- (button1width+10) / 2) && msg1.y > ((windowheight / 2 + 70) - (buttonheight-20)/2) && msg1.x < ((windowwidth / 2 - 85) + (button1width + 10) / 2) && msg1.y < ((windowheight / 2 + 70) + (buttonheight - 20) / 2))//若鼠标在按钮内
				    {
					    game();//启动游戏
				    }
					if (msg1.x > ((windowwidth / 2 + 85) - (button1width + 10) / 2) && msg1.y > ((windowheight / 2 + 70) - (buttonheight - 20) / 2) && msg1.x < ((windowwidth / 2 + 85) + (button1width + 10) / 2) && msg1.y < ((windowheight / 2 + 70) + (buttonheight - 20) / 2))
					{
						close = 1;
						closegraph();
						break;
					}
				    break;
				default:
					if (msg1.x > ((windowwidth / 2 - 85) - (button1width + 10) / 2) && msg1.y > ((windowheight / 2 + 70) - (buttonheight - 20) / 2) && msg1.x < ((windowwidth / 2 - 85) + (button1width + 10) / 2) && msg1.y < ((windowheight / 2 + 70) + (buttonheight - 20) / 2))
					{
						button2(windowwidth / 2 - 85, windowheight / 2 + 70, button1width + 10, buttonheight - 20, "重新开始", 30);						
					}
					else
					{
						if (msg1.x > ((windowwidth / 2 + 85) - (button1width + 10) / 2) && msg1.y > ((windowheight / 2 + 70) - (buttonheight - 20) / 2) && msg1.x < ((windowwidth / 2 + 85) + (button1width + 10) / 2) && msg1.y < ((windowheight / 2 + 70) + (buttonheight - 20) / 2))
						{
							button2(windowwidth / 2 + 85, windowheight / 2 + 70, button1width + 10, buttonheight - 20, "退出游戏", 30);
						}
						else
						{
							button1(windowwidth / 2 - 85, windowheight / 2 + 70, button1width + 10, buttonheight - 20, "重新开始", 30);
							button1(windowwidth / 2 + 85, windowheight / 2 + 70, button1width + 10, buttonheight - 20, "退出游戏", 30);
						}
						
					}

			}
		}
        EndBatchDraw();
	}

	getchar();
}

//倒计时
void beginingtimer()
{
	int i;
	for (i = 3; i >= 1; i--)
	{
		BeginBatchDraw();//帧开始

		cleardevice();//清屏

		refreshbackground();//加载背景

		//显示倒计时
		char number = '0' + i;
		setbkmode(TRANSPARENT);
		settextstyle(100, 0, "微软雅黑");//字体及大小
		settextcolor(WHITE);//颜色
		int length1 = textwidth(number) / 2;
		int height1 = textheight(number) / 2;
		outtextxy(windowwidth / 2 - length1, windowheight / 2 - height1, number);//居中显示

		//显示飞机
		putimage(player.x - playerplanewidth / 2, player.y - playerplaneheight / 2, &imgplayerplane0, NOTSRCERASE);//掩码图（使透明）
		putimage(player.x - playerplanewidth / 2, player.y - playerplaneheight / 2, &imgplayerplane1, SRCINVERT);

		EndBatchDraw();//帧结束

		Sleep(1000);//每秒刷新一次
	}
}


struct behavior
{

	//创建子弹
    void creatbullet()
    {
	    for (int i = 0; i < bulletnum; i++)
	    {
		    if (bullet[i].isalive == 0)
		    {
			    bullet[i].x = player.x-5;
			    bullet[i].y = player.y;
			    bullet[i].isalive = 1;
			    bulletspeed[i] = bulletinitialspeed;
				bulletexplosion[i].r = bulletexplosionradius;
				bulletexplosion[i].isalive = 0;
			    break;
	    	}
	    }
    }	
	//子弹运动
    void bulletmove()
    {
	    for (int i = 0; i < bulletnum; i++)
	    {
		    if (bullet[i].isalive==1)
		    {
			    bullet[i].y -= bulletspeed[i];
			    if(bulletspeed[i]>bulletminspeed)bulletspeed[i]-=bulletacc;
			    if (bullet[i].y <= 0)bullet[i].isalive = 0;
		    }
	    }
    }

    //玩家行为
    void playermove()
    {
	    //移动
	    if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W'))//当上键或W键按下
	    {
		    if (player.y > 0)//飞机在上边界内
		    {
			    if (speedup < maxspeed)speedup += accela;//且飞机速度不超过最大速度时，给飞机提供向上的加速度			
		    }
	    }
	    if (player.y > 0)player.y -= speedup;//飞机不超过窗口上边界时才能向上移动

    	if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S'))//下键
	    {
		    if (player.y < windowheight)
		    {
			     if(speeddown<maxspeed)speeddown += accela;						
		    }

    	}
	    if (player.y < windowheight)player.y += speeddown;
	
	    if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A'))//左键
	    {
		    if (player.x > 0)
		    {
			    if (speedleft < maxspeed)speedleft += accela;			
		    }
	    }
	    if (player.x > 0)player.x -= speedleft;
	
	    if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D'))//右键
	    {
		    if (player.x < windowwidth)
		    {
			    if (speedright < maxspeed)speedright += accela;			
		    }
	    }
	    if (player.x < windowwidth)player.x += speedright;
	
	    //无按键按下时加速度减小
	    if (speedup > 0)speedup-=accela2;
	    if (speeddown > 0)speeddown-=accela2;
	    if (speedleft > 0)speedleft-=accela2;
	    if (speedright > 0)speedright-=accela2;



	    //发射子弹
	    if (GetAsyncKeyState(VK_SPACE))//如果按下空格
	    {
		    for (int i = 0; i < bulletnum; i++)
		    {
		        if (timer1(bulletintervaltime))//时间差超过一定时间才发射子弹
		        {
                    behave.creatbullet();
		        }
		    }
		
	    }
    }
	//你炸了
	void playerexplode()
	{
		player.isalive = 0;

		gameover();
	}

	//敌机初始化
	void enemyinitialize(int i)
	{
		srand(rand()%1000);//给随机数播种
		if (rand() % 100 >= 20)//80的几率
		{
			enemy[i].type = 0;
			enemy[i].width = enemyplane0width;
			enemy[i].height = enemyplane0height;
			enemy[i].hp = 1;
			explosion[i].r = explosionradius0;
			explosion[i].isalive = 0;
		}
		else
		{
			enemy[i].type = 1;
			enemy[i].width = enemyplane1width;
			enemy[i].height = enemyplane1height;
			enemy[i].hp = 3;
			explosion[i].r = explosionradius1;
			explosion[i].isalive = 0;
		}	
	}
	//创建敌机
	void createnemy()
	{
		for (int i = 0; i < enemynum; i++)
		{
			if (enemy[i].isalive == 0)
			{
				enemy[i].isalive = 1;
				enemy[i].x = rand() % (windowwidth-40)+20;
				enemy[i].y = rand() % windowheight - windowheight - 80;
				if (enemy[i].type == 0)
				{
                    enemyplanespeed[i] = rand() % 15 / 10.0 + 0.6;
				}
				if (enemy[i].type == 1)
				{
					enemyplanespeed[i] = rand() % 8 / 10.0 + 0.6;
				}
				
				if (enemyplanespeed[i] < enemymaxspeed)enemyspeedup+=0.2;//每次创建的敌机速度都会快一点点
			}
		}
	}
	//敌机移动
	void enemymove()
	{
		for (int i = 0; i < enemynum; i++)
		{
			if (enemy[i].isalive == 1)
			{	
				if (enemy[i].type == 0)
			    {
				    enemy[i].y += enemyplanespeed[i]+enemyspeedup/10;
			    }
			    if (enemy[i].type == 1)
			    {
				    enemy[i].y += enemyplanespeed[i]+enemyspeedup/20;
			    }
			}
			if (enemy[i].y > windowheight + 100)//若超出边界
			{
				enemy[i].isalive = 0;
				enemyinitialize(i);//重新随机产生敌机类型

			}
		}
	}
	//敌机炸了
	void enemyexplode(int i)
	{
		if (explosion[i].isalive == 1)
		{

			if (explosion[i].r > 0)
			{
				explosion[i].r -= 10;
				enemy[i].y -= 5;
			}
			else
			{
				enemy[i].isalive = 0;//让它死
				enemyinitialize(i);//重新产生敌机
				explosion[i].isalive = 0;
			}
		}
		
	}

	//子弹炸了
	void bulletexplode(int i)
	{
		if (bulletexplosion[i].isalive == 1)
		{
			if (bulletexplosion[i].r > 0)bulletexplosion[i].r -= 5;
			else
			{
                bulletexplosion[i].isalive = 0;
			}
				
		}
	}

	//碰撞检测
	void collisiondetect()
	{
		for (int i = 0; i < enemynum; i++)//遍历敌机
		{
			if (enemy[i].isalive == 0)continue;//若敌机已死则不检测
			else
			{
				//检测子弹
				for (int j = 0; j < bulletnum; j++)
				{
                    bulletexplode(j);
					if (bullet[j].isalive == 0)continue;//若子弹已没则不检测
					else
					{
						if (bullet[j].x > enemy[i].x - enemy[i].width/2-5 && bullet[j].x < enemy[i].x + enemy[i].width/2+5 && bullet[j].y < enemy[i].y+10 && bullet[j].y>enemy[i].y-enemy[i].height && bullet[j].y > 0)
						{
							bullet[j].isalive = 0;
							enemy[i].y -= 5;
							enemy[i].hp--;
							bulletexplosion[j].isalive = 1;
						}
					}				
				}

				//检测玩家
				if (enemy[i].y + enemy[i].height / 2 > player.y - playerplaneheight / 2 && enemy[i].y - enemy[i].height / 2 < player.y + playerplaneheight / 2 && enemy[i].x + enemy[i].width / 2 > player.x - playerplanewidth / 2 && enemy[i].x - enemy[i].width / 2 < player.x + playerplanewidth / 2)//如果敌机和玩家碰撞
				{
					explosion[i].isalive = 1;
					player.hp -= (enemy[i].type+1)*10;
					enemy[i].y -= 10;
					speeddown = 4;
				}
			}
			if (enemy[i].hp <= 0)//血量小于等于0
			{
                explosion[i].isalive=1;
				if (enemy[i].type == 0)score += 1;
				if (enemy[i].type == 1)score += 3;
			}
			enemyexplode(i);
		}
	}

	//显示游戏信息
	void infodisplay()
	{
		//血量
		char textHP[] = "HP: ";
		char textHPvalue[4];
		itoa(player.hp, textHPvalue, 10);//将血量以十进制转化为字符串
		setbkmode(TRANSPARENT);
		settextstyle(30, 0, "微软雅黑");//字体及大小
		settextcolor(WHITE);//颜色
		outtextxy(10, 10, textHP);
		outtextxy(50, 10, textHPvalue);

		//分数
		char textscore[] = "分数: ";
		char textscorevalue[6];
		itoa(score, textscorevalue, 10);
		setbkmode(TRANSPARENT);
		settextstyle(30, 0, "微软雅黑");//字体及大小
		settextcolor(WHITE);//颜色
		outtextxy(10, 40, textscore);
		outtextxy(70, 40, textscorevalue);

		//生存时间
		 lastingtime= (clock()-start)/1000;
		char texttime[] = "时间: ";
		char texttimesec[3];
		char texttimemin[5];
		int sec, min;
		//排版
		if (lastingtime < 60)
		{
		    itoa(lastingtime, texttimesec, 10);
		    setbkmode(TRANSPARENT);
		    settextstyle(30, 0, "微软雅黑");//字体及大小
		    settextcolor(WHITE);//颜色
		    outtextxy(10, 70, texttime);
		    outtextxy(70, 70, texttimesec);
			if(lastingtime<10)outtextxy(85, 70, 's');
			else outtextxy(95, 70, 's');
		}
		else
		{
			sec = lastingtime % 60;
			min = lastingtime / 60;
			itoa(sec, texttimesec, 10);
			itoa(min, texttimemin, 10);
			setbkmode(TRANSPARENT);
			settextstyle(30, 0, "微软雅黑");//字体及大小
			settextcolor(WHITE);//颜色
			outtextxy(10, 70, texttime);
			if (min < 10)
			{
				outtextxy(70, 70, texttimemin);
				outtextxy(85, 70, "min");
				outtextxy(130, 70, texttimesec);
				if (sec < 10)outtextxy(150, 70, 's');
				else outtextxy(160, 70, 's');
			}
			else
			{
				outtextxy(70, 70, texttimemin);
				outtextxy(95, 70, "min");
				outtextxy(140, 70, texttimesec);
				if (sec < 10)outtextxy(160, 70, 's');
				else outtextxy(170, 70, 's');
			}
		}
	}
}behave;

//初始化游戏参数
void initialize()
{
	speedup = 0;
	speeddown = 0;
	speedleft = 0;
	speedright = 0;

	enemyspeedup = 0;

	score = 0;//初始化游戏分

	//玩家
	player.x = windowwidth / 2;//默认横向居中
	player.y = windowheight - 50;//默认在底部
	player.isalive = 1;//默认活着
	player.hp = 100;//血量初始值为100%

	//子弹
	for (int i = 0; i < bulletnum; i++)
	{
		bullet[i].isalive = 0;
		bullet[i].x = 0;//一开始在画面外
		bullet[i].y = 0;
	}

	//敌机及爆炸
	for (int i = 0; i < enemynum; i++)
	{
		enemy[i].isalive = 0;
		behave.enemyinitialize(i);
		enemyplanespeed[i] = 0;
		explosion[i].isalive = 0;
	}

	//初始化滚动背景的位置
	bkg1.y = 0;
	bkg2.y = -windowheight;
}


//绘制游戏画面（每帧）
void drawgame()
{
	cleardevice();//清屏

	refreshbackgroundscroll();//加载滚动背景

	//画子弹
	for (int i = 0; i < bulletnum; i++)
	{
		if (bullet[i].isalive == 1)//如果子弹存在
		{
            putimage(bullet[i].x, bullet[i].y, &imgbullet[0], NOTSRCERASE);//掩码图（使透明）
		    putimage(bullet[i].x, bullet[i].y, &imgbullet[1], SRCINVERT);
		}	
		if (bulletexplosion[i].isalive == 1)
		{
			setfillcolor(pink);
			solidcircle(bullet[i].x+5, bullet[i].y, bulletexplosion[i].r);
		}
	}

	//画玩家飞机
	if (player.isalive == 1)
	{
        putimage(player.x - playerplanewidth / 2, player.y - playerplaneheight / 2, &imgplayerplane0,NOTSRCERASE);//掩码图（使透明）
	    putimage(player.x - playerplanewidth / 2, player.y - playerplaneheight / 2, &imgplayerplane1,SRCINVERT);
	}
	
	//画敌机
	for (int i = 0; i < enemynum; i++)
	{
		if (enemy[i].isalive == 1)//若敌机存在
		{
			if (enemy[i].type == 0)
			{
				putimage(enemy[i].x - enemyplane0width / 2, enemy[i].y - enemyplane0height / 2, &imgenemyplane[0][0], NOTSRCERASE);
				putimage(enemy[i].x - enemyplane0width / 2, enemy[i].y - enemyplane0height / 2, &imgenemyplane[0][1], SRCINVERT);
			}
			if (enemy[i].type == 1)
			{
				putimage(enemy[i].x - enemyplane1width / 2, enemy[i].y - enemyplane1height / 2, &imgenemyplane[1][0], NOTSRCERASE);
				putimage(enemy[i].x - enemyplane1width / 2, enemy[i].y - enemyplane1height / 2, &imgenemyplane[1][1], SRCINVERT);
			}
			if (explosion[i].isalive == 1)
			{
				setfillcolor(YELLOW);
				solidcircle(enemy[i].x, enemy[i].y, explosion[i].r);
			}
		}
	}

	//画游戏信息（血量 分数）
	behave.infodisplay();
}



//游戏
void game()
{
	cleardevice();//清屏

	//背景
	refreshbackground();

	initialize();//初始化游戏参数

    loadimg();

	beginingtimer();//倒计时

	start = clock();//开始计时

	while (close==0)
	{

		behave.collisiondetect();//碰撞检测

		BeginBatchDraw();//开始绘制
		drawgame();//绘制
		EndBatchDraw();//结束绘制（即显示）

		behave.playermove();//玩家移动
		behave.bulletmove();//子弹移动


		if (timer2(500))behave.createnemy();//每500ms产生一个敌机

		
		behave.enemymove();//敌机移动

		if (player.hp <= 0)behave.playerexplode();

		Sleep(20);//每隔20ms渲染一帧（不包括渲染时间）
	}

}


//启动界面
void startup()
{
	//建立窗口
	initgraph(windowwidth, windowheight);

	//设置背景颜色	
	setbkcolor(BLACK);
	cleardevice();

	//设置背景图
	IMAGE imgbkg;//启动页背景图
	loadimage(&imgbkg, "./pictures/background.png");
	putimage(0, 0, &imgbkg);

	//设置按钮1
	button1(windowwidth / 2, windowheight / 2, button1width, buttonheight, "开始",50);
	//设置按钮2
	button2(windowwidth / 2, windowheight / 2, button1width, buttonheight, "开始",50);
	
	
	//设置键鼠响应
	while (close==0)//一直接收消息
	{

        BeginBatchDraw();//帧开始
		ExMessage msg1;//定义一个消息
		if (peekmessage(&msg1, EM_MOUSE))//如果有鼠标消息
		{
			switch (msg1.message)
			{
			    case WM_LBUTTONDOWN://左键按下时
				    if (msg1.x > (windowwidth - button1width) / 2 && msg1.y > (windowheight - buttonheight) / 2 && msg1.x < (windowwidth + button1width) / 2 && msg1.y < (windowheight + buttonheight) / 2)//若鼠标在按钮内
				    {
					    game();//启动游戏
				    }
				    break;
				default:
					if (msg1.x > (windowwidth - button1width) / 2 && msg1.y > (windowheight - buttonheight) / 2 && msg1.x < (windowwidth + button1width) / 2 && msg1.y < (windowheight + buttonheight) / 2)//若鼠标在按钮内
					{
						button2(windowwidth / 2, windowheight / 2, button1width, buttonheight, "开始",50);
					}
					else
					{
						button1(windowwidth / 2, windowheight / 2, button1width, buttonheight, "开始",50);
					}

			}
		}
        EndBatchDraw();//帧结束
	}


	closegraph();

}


//主函数
void main()
{
	srand((unsigned)time(NULL));//用时间播种随机数
	startup();
}
