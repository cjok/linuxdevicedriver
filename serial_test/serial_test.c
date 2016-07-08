#include<stdio.h>      
#include<stdlib.h>    
#include<unistd.h>   
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>    
#include<termios.h> 
#include<errno.h>    
#include<string.h>  
   
   
#define FALSE  -1  
#define TRUE   0  
   
int serial_open(int fd,char* port)  
{  
     
    fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);  
	if (FALSE == fd) {  
		perror("Can't Open Serial Port");  
        return(FALSE);  
    }  
	
	//恢复串口为阻塞状态
    if(fcntl(fd, F_SETFL, 0) < 0) {  
        printf("fcntl failed!\n");  
        return(FALSE);  
	} else {  
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));  
    }  

	//测试是否为终端设备
    if(0 == isatty(STDIN_FILENO)) {  
        printf("standard input is not a terminal device\n");  
		return(FALSE);  
	} else {  
		printf("isatty success!\n");  
	}                

	printf("fd->open=%d\n",fd);  
	return fd;  
}  
  
void serial_close(int fd)  
{  
    close(fd);  
}  

/******************************************************************* 
* 名称：                UART0_Set 
* 功能：                设置串口数据位，停止位和效验位 
* 入口参数：        fd        串口文件描述符 
*                              speed     串口速度 
*                              flow_ctrl   数据流控制 
*                           databits   数据位   取值为 7 或者8 
*                           stopbits   停止位   取值为 1 或者2 
*                           parity     效验类型 取值为N,E,O,,S 
*出口参数：          正确返回为1，错误返回为0 
*******************************************************************/    
int serial_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)  
{  
     
    int   i;  
	int   status;  
    int   speed_arr[] = {B921600, B115200, B19200, B9600, B4800, B2400, B1200, B300};  
    int   name_arr[] = {921600, 115200,  19200,  9600,  4800,  2400,  1200,  300};  
           
    struct termios options;  
	
     /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
	 *该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，
	 *若调用失败，函数返回值为1. 
    */ 
    if  ( tcgetattr( fd,&options)  !=  0) {  
          perror("SetupSerial 1");      
          return(FALSE);   
    }  
    //设置串口输入波特率和输出波特率  
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {  
        if  (speed == name_arr[i]) {               
            cfsetispeed(&options, speed_arr[i]);   
            cfsetospeed(&options, speed_arr[i]);    
        }  
	}       
    //修改控制模式，保证程序不会占用串口 
    options.c_cflag |= CLOCAL;  
	//修改控制模式，使得能够从串口中读取输入数据  
    options.c_cflag |= CREAD;  
	
    //设置数据流控制
    switch(flow_ctrl) 
    {  
        
       case 0 ://不使用流控制 
              options.c_cflag &= ~CRTSCTS;  
              break;     
        
       case 1 ://使用硬件流控制  
              options.c_cflag |= CRTSCTS;  
              break;  
       case 2 ://使用软件流控制
              options.c_cflag |= IXON | IXOFF | IXANY;  
              break;  
    }  
	//设置数据位  
    //屏蔽其他标志位 
    options.c_cflag &= ~CSIZE;  
    switch (databits)  
    {    
       case 5    :  
                     options.c_cflag |= CS5;  
                     break;  
       case 6    :  
                     options.c_cflag |= CS6;  
                     break;  
       case 7    :      
                 options.c_cflag |= CS7;  
                 break;  
       case 8:      
                 options.c_cflag |= CS8;  
                 break;    
       default:     
                 fprintf(stderr,"Unsupported data size\n");  
                 return (FALSE);   
    }  
	
	//设置校验位
    switch (parity)  
    {    
       case 'n':  //无奇偶校验位
       case 'N': 
                 options.c_cflag &= ~PARENB;   
                 options.c_iflag &= ~INPCK;      
                 break;   
       case 'o':    //设置为奇校验
       case 'O':
                 options.c_cflag |= (PARODD | PARENB);   
                 options.c_iflag |= INPCK;               
                 break;   
       case 'e':   //设置为偶校验
       case 'E':
                 options.c_cflag |= PARENB;         
                 options.c_cflag &= ~PARODD;         
                 options.c_iflag |= INPCK;        
                 break;  
       case 's':  //设置为空格  
       case 'S': 
                 options.c_cflag &= ~PARENB;  
                 options.c_cflag &= ~CSTOPB;  
                 break;   
        default:    
                 fprintf(stderr,"Unsupported parity\n");      
                 return (FALSE);   
    }   
	// 设置停止位 
    switch (stopbits) {    
       case 1:     
            options.c_cflag &= ~CSTOPB; break;   
       case 2:     
            options.c_cflag |= CSTOPB; break;  
       default:     
            fprintf(stderr,"Unsupported stop bits\n");   
			return (FALSE);  
    }  
     //修改输出模式，原始数据输出  
	options.c_oflag &= ~OPOST;  
	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);  
     
    //设置等待时间和最小接收字符  
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */    
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */  
     
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读  
    tcflush(fd,TCIFLUSH);  
     
    //激活配置 (将修改后的termios数据设置到串口中）  
    if (tcsetattr(fd,TCSANOW,&options) != 0) {  
		perror("com set error!\n");    
        return (FALSE);   
    }  
	
    return (TRUE);   
}  

/******************************************************************* 
* 名称：                   
* 功能：                接收串口数据 
* 入口参数：        fd                  :文件描述符     
*                              rcv_buf     :接收串口中数据存入rcv_buf缓冲区中 
*                              data_len    :一帧数据的长度 
* 出口参数：        正确返回为1，错误返回为0 
*******************************************************************/     
int serial_recv(int fd, char *rcv_buf,int data_len)  
{  
    int len,fs_sel;  
    fd_set fs_read;  
     
    struct timeval time;  
     
    FD_ZERO(&fs_read);  
    FD_SET(fd,&fs_read);  
     
    time.tv_sec = 10;  
    time.tv_usec = 0;  
    
	//使用select实现串口的多路通信 
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);  
    if(fs_sel) {  
        len = read(fd,rcv_buf,data_len);  
        printf("I am right!(version1.2) len = %d fs_sel = %d\n",len,fs_sel);  
		return len;  
    } else {  
        printf("Sorry,I am wrong!");  
		return FALSE;  
	}       
}  

/******************************************************************** 
* 名称：                  UART0_Send 
* 功能：                发送数据 
* 入口参数：        fd                  :文件描述符     
*                              send_buf    :存放串口发送数据 
*                              data_len    :一帧数据的个数 
* 出口参数：        正确返回为1，错误返回为0 
*******************************************************************/  
int serial_send(int fd, char *send_buf,int data_len)  
{  
    int len = 0;  
     
    len = write(fd,send_buf,data_len);  
    if (len == data_len ) {  
		return len;  
	} else {  
        tcflush(fd,TCOFLUSH);  
        return FALSE;  
    }  
     
}     
   
int main(void)  
{  
    int fd;                     
    int err;                   
    int len;                          
    int i;  
    char rcv_buf[100];         
    //char send_buf[100]="3A 16 24 01 0B 46 00 0D 0A";  
    char send_buf[100]="PUALABCD";  

    fd = serial_open(fd, "/dev/ttyMT0"); 
    do{  
		//串口初始化
        err = serial_set(fd,921600,0,8,1,'N');  
		printf("Set Port Exactly!\n");  
    }while(FALSE == err || FALSE == fd);  
	
 #if 1    
    for(i = 0;i < 10;i++) {  
		len = serial_send(fd,send_buf,10);  
		if(len > 0)  
			printf(" %d send data successful\n",i);  
		else  
			printf("send data failed!\n");  
	
		sleep(2);  
	}  
#endif


	while (1){    
		len = serial_recv(fd, rcv_buf,9);  
		if(len > 0) {  
			rcv_buf[len] = '\0';  
			printf("receive data is %s\n",rcv_buf);  
			printf("len = %d\n",len);  
		} else {  
			printf("cannot receive data\n");  
		}  
		sleep(2);  
	}              
    serial_close(fd);   

	return 0;
}  
    
