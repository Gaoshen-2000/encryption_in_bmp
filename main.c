#include<stdio.h>
#include<string.h>
#include<math.h>

int main(void)
{
    //相关说明
    printf( "Please be sure the files are in the same directory of this program.\n" );
    printf( "Press Enter to continue\n" );
    getchar();
    //读取位图，19778判断大小，判断是否为24位位图，
    char name_of_bitmap[100];
    unsigned short fileType;//由特定代码判断是否为bmp图像
    unsigned int offset;//判断偏移量是否符合要求，为54
    int width;//图像宽度
    int height;//图像高度
    int maxsize;//宽*高*3
    unsigned short bit_count;//仅支持24位位图

    unsigned short flag;
    printf( "Enter the name of the .bmp file.\n" );
    scanf( "%s" , name_of_bitmap );
    //可自动化添加后缀
    //仅在位图名称中使用，文件名称后缀未知，不进行类似操作
    //此处判断应使用strchr对.进行判断
    //若判断".bmp"，则举例如下：
    //若输入为a.docx,自动添加后缀后输出有问题，应为不是bitmap，实为不能打开
    if( strchr( name_of_bitmap , '.' ) == NULL )
    {
        strcat( name_of_bitmap , ".bmp" );
    }
    FILE * fp_bitmap = fopen( name_of_bitmap , "rb+" );
    if( fp_bitmap == NULL )
    {
        printf( "Unable to open the bitmap.\n" );
        getchar();
        getchar();
        return -1;
    }
    fread( &fileType , 1 , sizeof(unsigned short) , fp_bitmap );
    if ( fileType != 0x4d42 )
    {
        printf( "It's not a bitmap file.\n" );
        getchar();
        getchar();
        return -1;
    }
    fseek( fp_bitmap , 8 , SEEK_CUR );
    fread( &offset , 1 , sizeof(unsigned int) , fp_bitmap );
    if( offset != 54 )
    {
        printf( "Something wrong with the bitmap.\n" );
        getchar();
        getchar();
        return -1;
    }
    fseek( fp_bitmap , 4 , SEEK_CUR );
    fread( &width , 1 , sizeof(int) , fp_bitmap );
    fread( &height , 1 , sizeof(int) , fp_bitmap );
    maxsize = width * height * 3;
    fseek( fp_bitmap , 2 , SEEK_CUR );
    fread( &bit_count , 1 , sizeof(unsigned short) , fp_bitmap);
    if( bit_count != 24 )
    {
        printf( "Please use a 24_bit bitmap.\n" );
        getchar();
        getchar();
        return -1;
    }
    fseek( fp_bitmap , 24 , SEEK_CUR );


    printf( "Do you want to encrypt or decrypt?\n" );
    printf( "1.Encrypt 2.Decrypt\n" );
    printf( "Enter your choise:" );
    scanf("%d",&flag);

    char name_of_file[100];
    FILE * fp_file = NULL;
    int size = 0;
    int cnt = 1 + (int)log2(maxsize);
    unsigned char pixel_color;
    int index;
    int byte_with_bit[8];
    unsigned char byte_file = 0;

    if( flag == 1 )
    {
        printf( "Enter the filename you want to encrypt:\n" );
        scanf("%s",name_of_file);
        fp_file = fopen( name_of_file , "rb" );
        if( fp_file == NULL )
        {
            printf( "Unable to open the file.\n" );
            getchar();
            getchar();
            return -1;
        }
        fseek( fp_file , 0 , SEEK_END );
        size = 8 * ftell(fp_file);
        fseek( fp_file , 0 , SEEK_SET );
        if( size >= maxsize - 32 )
        {
            printf("The file is too big / The bitmap is too small.\n");
            getchar();
            getchar();
            return -1;
        }
        int saveCnt[cnt];
        for( int i = cnt - 1 ; i >= 0 ; i -- )
        {
            saveCnt[i] = size % 2;
            size /= 2;
        }
        for( int i = 0 ; i < cnt ; i ++ )
        {
            fread( &pixel_color , sizeof(unsigned char) , 1 , fp_bitmap );
            if( ( saveCnt[i] - pixel_color ) % 2 != 0 )
            {
                fseek( fp_bitmap , - 1 , SEEK_CUR );
                if( pixel_color % 2 == 0 )
                    pixel_color ++;
                else
                    pixel_color --;
                fwrite( &pixel_color , sizeof(unsigned char) , 1 , fp_bitmap );
                fflush(fp_bitmap);
            }
        }
        index = 8;
        while(1)
        {
            fread( &pixel_color , sizeof(unsigned char) , 1 , fp_bitmap );
            if( index == 8 )
            {
                fread( &byte_file , sizeof(unsigned char) , 1 , fp_file );
                if(feof(fp_file))
                    break;
                for( int j = 7 ; j >= 0 ; j -- )
                {
                    byte_with_bit[j] = byte_file % 2;
                    byte_file /= 2;
                }
                index = 0;
            }
            if( ( byte_with_bit[index] - pixel_color ) % 2 != 0 )
            {
                fseek( fp_bitmap , - 1 , SEEK_CUR );
                if( pixel_color % 2 == 0 )
                    pixel_color ++;
                else                        
                    pixel_color --;
                fwrite( &pixel_color , sizeof(unsigned char) , 1 , fp_bitmap );
                fflush(fp_bitmap);
            }
            index ++;
        }
    }
    else if( flag == 2 )
    {
        printf( "Enter the expected output file name:\n" );
        scanf("%s",name_of_file);
        fp_file = fopen( name_of_file , "wb" );
        if( fp_file == NULL )
        {
            printf( "Fail to open the file.\n" );
            getchar();
            getchar();
            return -1;
        }
        fseek( fp_file , 0 , SEEK_SET );
        size = 0;
        for( int i = 0 ; i < cnt ; i ++ )
        {
            fread( &pixel_color , sizeof(unsigned char) , 1 , fp_bitmap );
            size = 2 * size + pixel_color % 2;
        }
        index = 0;
        while(size --)
        {
            fread( &pixel_color , sizeof(unsigned char) , 1 , fp_bitmap );
            byte_with_bit[index] = pixel_color % 2;
            index ++;
            if( index == 8 )
            {
                for( int j = 0 ; j < 8 ; j ++)
                    byte_file = 2 * byte_file + byte_with_bit[j];
                fwrite( &byte_file , sizeof(unsigned char) , 1 , fp_file );
                //每次循环八位，高位自动被替换
                //byte_file = 0;
                fflush(fp_file);
                index = 0;
            }
        }
    }
    else
    {
        printf("Please enter a right code.\n");
        getchar();
        getchar();
        return -1;
    }
    fclose( fp_bitmap );
    fclose( fp_file );
    printf( "Task success.\n" );
    getchar();
    getchar();
    return 0;
}