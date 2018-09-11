#include "key.h"
#include "sys.h"
#include "delay.h"

t_keys_fifo keys_fifo;       //��������FIFO

t_keys keys[KEY_ID_MAX];


/***********************************************************************
* �������ƣ� get_key1_state
* �������ܣ� ��ȡ������״̬
* ���������  ��
* �� �� ֵ��   �������·���1 ���򷵻�0
* ����˵���� �������·���PRESS ���򷵻�LIFT
****************************************************************************/
static uint8_t get_key1_state(void)
{
    uint8_t res  = GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_Pin);  //��ƽ̨��ֲ���
    if(res == 0)
    {
        return PRESS;   //��������
    }
    else
    {
        return LIFT;    //����̧��
    }
}




/***********************************************************************
* �������ƣ� key_in_fifo
* �������ܣ� ��һ������״̬����FIFO��
* ���������  ���尴��״̬
* �� ��  ֵ�� ��
* ����˵������
****************************************************************************/
void key_in_fifo(e_keys_status key_state)
{
    keys_fifo.fifo_buffer[keys_fifo.write] = key_state;

    if (++keys_fifo.write  >= KEY_FIFO_SIZE)
    {
        keys_fifo.write = 0;
    }
}

/***********************************************************************
* �������ƣ� key_out_fifo
* �������ܣ� �Ӱ���FIFO��ȡ��һ�������¼�
* ���������  ��
* �� ��  ֵ��  ��ǰFIFO�л���İ����¼�
* ����˵������
****************************************************************************/
e_keys_status key_out_fifo(void)
{
    e_keys_status ret;

    if (keys_fifo.read == keys_fifo.write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = keys_fifo.fifo_buffer[keys_fifo.read];

        if (++keys_fifo.read >= KEY_FIFO_SIZE)
        {
            keys_fifo.read = 0;
        }
        return ret;
    }
}


/***********************************************************************
* �������ƣ� get_key_state
* �������ܣ� ��ȡ����״̬
* ���������  ����ID
* �� ��  ֵ��  ����״̬ [ PRESS : �������� LIFT : ����̧��]
* ����˵����   ��
****************************************************************************/
uint8_t get_key_state(e_keys_id key_id)
{
    return keys[key_id].state;
}

/***********************************************************************
* �������ƣ� set_keys_param
* �������ܣ� ���ð����ṹ���еĲ���
* ���������  key_id[IN] : ����ID
                     long_time[IN] :  ����ʱ��*ms  0��ʾ��֧�ֳ���
                     repeat_speed[IN] : �����ٶ� ms 0��ʾ��֧������
* �� ��  ֵ��  ��
* ����˵����   ��
****************************************************************************/
void set_keys_param(e_keys_id key_id, uint16_t long_time, uint8_t  repeat_speed)
{
    keys[key_id].long_time = long_time;			/* ����ʱ�� 0 ��ʾ����ⳤ�����¼� */
    keys[key_id].repeat_speed = repeat_speed;			/* �����������ٶȣ�0��ʾ��֧������ */
    keys[key_id].repeat_count = 0;						/* �������� �� ���� */
}


/***********************************************************************
* �������ƣ� clear_keys_fifo
* �������ܣ� �������������
* ���������  ��
* �� ��  ֵ��  ��
* ����˵����   ��
****************************************************************************/
void clear_keys_fifo(void)
{
    keys_fifo.read = keys_fifo.write;
}

/***********************************************************************
* �������ƣ� keys_hardware_init
* �������ܣ� ��ʼ��������Ӧ��IO��
* ���������  ��
* �� ��  ֵ��  ��
* ����˵����   ��
****************************************************************************/
static void keys_hardware_init(void)
{

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(KEY1_RCC, ENABLE);//ʹ��GPIOʱ��

    GPIO_InitStructure.GPIO_Pin = KEY1_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  //����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(KEY1_GPIO, &GPIO_InitStructure);//��ʼ��GPIO

}

/***********************************************************************
* �������ƣ� keys_value_init
* �������ܣ� ��ʼ�������ṹ���е���ر���
* ���������  ��
* �� ��  ֵ��  ��
* ����˵����   ��
****************************************************************************/
static void keys_value_init(void)
{
    uint8_t i;

    /* �԰���FIFO��дָ������ */
    keys_fifo.read = 0;
    keys_fifo.write = 0;


    /* ��ÿ�������ṹ���Ա������һ��ȱʡֵ */
    for (i = 0; i < KEY_ID_MAX; i++)
    {
        keys[i].long_time = KEY_LONG_TIME;			/* ����ʱ�� 0 ��ʾ����ⳤ�����¼� */
        keys[i].count = KEY_FILTER_TIME ;		/* ����������Ϊ�˲�ʱ�� */
        keys[i].state = LIFT;							            /* ����ȱʡ״̬��0Ϊδ���� */
        keys[i].repeat_speed = KEY_REPEAT_TIME;						/* �����������ٶȣ�0��ʾ��֧������ */
        keys[i].repeat_count = 0;						/* ���������� */
        keys[i].double_count = 0;                      /* ˫��������*/

        keys[i].short_key_down = NULL;          /* �������»ص�����*/
        keys[i].skd_arg = NULL;                     /* �������»ص���������*/
        keys[i].short_key_up = NULL;            /* ����̧��ص�����*/
        keys[i].sku_arg = NULL;                     /* ����̧��ص���������*/
        keys[i].long_key_down = NULL;         /* ���������ص�����*/
        keys[i].lkd_arg = NULL;                      /* ���������ص���������*/
        keys[i].double_key_down = NULL;
        keys[i].dkd_arg = NULL;

        keys[i].get_key_status = get_key1_state;    /* ��ȡ����״̬������ �����ƽ̨�й�*/

        keys[i].report_flag = KEY_REPORT_DOWN | KEY_REPORT_UP | KEY_REPORT_LONG | KEY_REPORT_DOUBLE;

    }

}

/***********************************************************************
* �������ƣ� detect_key
* �������ܣ� ���һ��������״̬����״ֵ̬����FIFO��
* ���������  key_id[IN] : ����ID
* �� ��  ֵ��  ��
* ����˵����  Ӧ�÷���һ��1ms�����ں�����
****************************************************************************/
static void detect_key(e_keys_id key_id)
{
    t_keys *p_key;

    p_key = &keys[key_id];  //��ȡ�����¼��ṹ��
    if (p_key->get_key_status() == PRESS)  //��������Ѿ�������
    {
        if (p_key->count < KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if(p_key->count < 2 * KEY_FILTER_TIME)
        {
            p_key->count+=KEY_TICKS;  //�˲� ʵ���˲�ʱ��Ϊ(KEY_FILTER_TIME+1)������
        }
        else
        {
            if (p_key->state == LIFT)
            {
                p_key->state = PRESS;

                if(p_key->report_flag&KEY_REPORT_DOWN)  //��������˰��������ϱ�����
                {
                    /* ���Ͱ�ť���µ���Ϣ */
                    key_in_fifo((e_keys_status)(KEY_STATUS * key_id + 1));   //���밴�������¼�
                }
                if(p_key->short_key_down)   //���ע���˻ص����� ��ִ��
                {
                    p_key->short_key_down(p_key->skd_arg);
                }
            }

            if (p_key->long_time > 0)     //�����Ҫ�������
            {
                if (p_key->long_count < p_key->long_time)
                {
                    /* ���Ͱ�ť�������µ���Ϣ */
                    if ((p_key->long_count+=KEY_TICKS) == p_key->long_time)
                    {
                        if(p_key->report_flag&KEY_REPORT_LONG)
                        {
                            /* ��ֵ���밴��FIFO */
                            key_in_fifo((e_keys_status)(KEY_STATUS * key_id + 3));  //���볤���¼�
                        }
                        if(p_key->long_key_down)        //��������˻ص�����
                        {
                            p_key->long_key_down(p_key->lkd_arg);   //ִ�лص�����
                        }
                    }
                }
                else
                {
                    if (p_key->repeat_speed > 0)      //��������������¼�
                    {
                        if ((p_key->repeat_count  += KEY_TICKS) >= p_key->repeat_speed)
                        {
                            p_key->repeat_count = 0;
                            /*����������ÿ��repeat_speed����1������ */
                            key_in_fifo((e_keys_status)(KEY_STATUS * key_id + 1));
                        }
                    }
                }
            }
        }
    }
    else    //�����ɿ�
    {
        if(p_key->count > KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if(p_key->count != 0)
        {
            //�����ɿ��˲�
            p_key->count-=KEY_TICKS;
        }
        else
        {
            //�˲�����
            if (p_key->state == PRESS)
            {
                p_key->state = LIFT;  //����̧��

                if(p_key->report_flag&KEY_REPORT_UP)
                {
                    /* ���Ͱ�ť�������Ϣ */
                    key_in_fifo((e_keys_status)(KEY_STATUS * key_id + 2));
                }
                if(p_key->short_key_up) //��������˻ص�����
                {
                    p_key->short_key_up(p_key->sku_arg);
                }
                //�������˫��Ҫ��
                if((p_key->double_count > KEY_DOUBLE_MIN)&&(p_key->double_count < KEY_DOUBLE_MAX))
                {
                    p_key->double_count = 0;
                    if(p_key->report_flag&KEY_REPORT_DOUBLE)    //���������ϱ�˫����־
                    {
                        key_in_fifo((e_keys_status)(KEY_STATUS * key_id + 4));  //�ϱ�˫���¼�
                    }
                    if(p_key->double_key_down)  //��������˻ص�����
                    {
                        p_key->double_key_down(p_key->dkd_arg); //ִ�лص�����
                    }
                }
                else
                {
                    //������˫��Ҫ�� ���������
                    p_key->double_count = 0;
                }

            }
            p_key->double_count += KEY_TICKS ;  //˫���¼�����

        }

        p_key->long_count = 0;  //������������
        p_key->repeat_count = 0;  //�ظ����ͼ�������
    }
}

/***********************************************************************
* �������ƣ� key_scan
* �������ܣ� ����ɨ��
* ��������� ��
* �� ��  ֵ��  ��
* ����˵����  Ӧ�÷���һ��1ms�����ں�����
****************************************************************************/
void key_scan(void)
{
    uint8_t i;

    for (i = 0; i < KEY_ID_MAX; i++)
    {
        detect_key(i);
    }
}


/***********************************************************************
* �������ƣ� keys_init
* �������ܣ� ����ģ���ʼ��
* ���������  ��
* �� ��  ֵ�� ��
* ����˵���� ϵͳ��ʼ��ʱӦ�����ô˺���
****************************************************************************/
void keys_init(void)
{
    keys_value_init();		        /* ��ʼ���������� */
    keys_hardware_init();		/* ��ʼ������Ӳ�� */
}


