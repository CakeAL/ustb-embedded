//-------------------DMA TESTING -----------------//
/******************************************************
ʵ�����ݣ� �趨DMA
����⵽����S1ʱ���趨DMA��RAM��ת�����ݣ�����ɹ�����
����LED3����c��������ʾ Correct�� ������ʾ Error
******************************************************/

#define LED1 P1_0 // ����P1.0��ΪLED1���ƶ�
#define LED2 P1_1 // ����P1.1��ΪLED2���ƶ�
#define LED3 P1_4 // ����P1.4��ΪLED3���ƶ�

#define KEY1 P0_1 // P0.1�ڿ��ư���KEY1

#define HAL_DMA_U0DBUF 0x70C1 // ����DMAĿ�ĵ�

#include "hal.h"
#include "hal_types.h"
#include <string.h>
char titleString[] = "-------- DMA Testing --------";
char hintString[] = "...Push KEY1 to start DMA ...";
char goodString[] = "...yes, DMA transfer correct";
char badString[] = "...bad,DMA transfer Error";
char revbuf; // �����ַ�

DMA_DESC dmaChannel;

/****************************************************************************
 * ��    ��: LedOnOrOff()
 * ��    ��: ������Ϩ������LED��
 * ��ڲ���: modeΪ0ʱLED����  modeΪ1ʱLED����
 * ���ڲ���: ��
 ****************************************************************************/
void LedOnOrOff(uint8 mode)
{
    LED1 = mode;
    LED2 = mode;
    LED3 = mode;
}

/****************************************************************************
 * ��    ��: InitLed()
 * ��    ��: ����LED��Ӧ��IO��
 * ��ڲ���: ��
 * ���ڲ���: ��
 ****************************************************************************/
void InitLed(void)
{

    P1DIR |= 0x01; // P1.0����Ϊ�����
    P1DIR |= 0x02; // P1.1����Ϊ�����
    P1DIR |= 0x10; // P1.4����Ϊ�����
    asm("NOP");

    LedOnOrOff(0); // ʹ����LED��Ĭ��ΪϨ��״̬
}

/****************************************************************************
 * ��    ��: InitKey()
 * ��    ��: ���ð�����Ӧ��IO��
 * ��ڲ���: ��
 * ���ڲ���: ��
 ****************************************************************************/
void InitKey(void)
{

    P0SEL &= ~0x02; // ����P0.1Ϊ��ͨIO��
    P0DIR &= ~0x02; // ��������P0.1���ϣ���P0.1Ϊ����ģʽ
    P0INP &= ~0x02; // ��P0.1��������
}

/****************************************************************************
 * ��    ��: KeyScan()
 * ��    ��: ��ȡ����״̬
 * ��ڲ���: ��
 * ���ڲ���: 0Ϊ̧��   1Ϊ��������
 ****************************************************************************/
unsigned char KeyScan(void)
{
    if (KEY1 == 0)
    {
        halWait(20);
        if (KEY1 == 0)
        {
            while (!KEY1)
                ;     // ���ּ��
            return 1; // �а�������
        }
    }

    return 0; // �ް�������
}

/****************************
//IO��ʼ������
*****************************/
void Initial_IO(void)
{
    InitLed();
    InitKey();
}

void initDma(void);
/****************************************************************************
 * ��    ��: InitUart()
 * ��    ��: ���ڳ�ʼ������
 * ��ڲ���: ��
 * ���ڲ���: ��
 ****************************************************************************/
void InitUart(void)
{
    PERCFG = 0x00;  // ������ƼĴ��� USART 0��IOλ��:0ΪP0��λ��1
    P0SEL = 0x0c;   // P0_2,P0_3�������ڣ����蹦�ܣ�
    P2DIR &= ~0xC0; // P0������ΪUART0

    U0CSR |= 0x80; // ����ΪUART��ʽ
    U0GCR |= 11;   // baud_e
    U0BAUD |= 216; // ��������Ϊ115200
    UTX0IF = 0;    // UART0 TX�жϱ�־��ʼ��λ0
    U0CSR |= 0x40; // �������
    IEN0 |= 0x84;  // ���ж�
}

/****************************************************************************
 * ��    ��: UartSendString()
 * ��    ��: ���ڷ��ͺ���
 * ��ڲ���: Data:���ͻ�����   len:���ͳ���
 * ���ڲ���: ��
 ****************************************************************************/
void UartSendString(char *Data, int len)
{
    uint16 i;

    for (i = 0; i < len; i++)
    {
        U0DBUF = *Data++;
        while (UTX0IF == 0)
            ;
        UTX0IF = 0;
    }
    U0DBUF = 0x0d;
    while (UTX0IF == 0)
        ;
    UTX0IF = 0;
    U0DBUF = 0x0a;
    while (UTX0IF == 0)
        ;
    UTX0IF = 0;
    halWait(600);
}

/****************************************************************************
 * ��    ��: UART0RX_ISR()
 * ��    ��: �����жϴ���
 * ��ڲ���: ��
 * ���ڲ���: ��
 ****************************************************************************/
char RxBuf;
#pragma vector = URX0_VECTOR
__interrupt void UART0RX_ISR(void)
{
    revbuf = U0DBUF;
    DMA_SET_ADDR_DESC0(&dmaChannel);
    DMA_ABORT_CHANNEL(0);
    DMA_ARM_CHANNEL(0);
    DMAIRQ = 0x00;
    DMA_START_CHANNEL(0);
    URX0IF = 0;
}

/******************************************************************************
 * @fn  initDma
 *
 * @brief
 *      Initializes components for the DMA transfer application example.
 ******************************************************************************/
void initDma(void)
{
    SET_MAIN_CLOCK_SOURCE(CRYSTAL);

    Initial_IO();
    InitUart(); // ���ô��ڳ�ʼ������

    UartSendString(&titleString[0], sizeof(titleString)); // ��ʾTitle�ַ���
}

/******************************************************************************
 * @fn  dma_main
 *
 * @brief
 *      Sets up the DMA to transfer data between to RAM locations, trigged by
 *      external interrupt generated by button S1. Checks validity of data
 *      after transfer.
 ******************************************************************************/
void main(void)
{

    initDma();

    // Clearing the destination
    // memset(destString, 0, sizeof(destString));
    // Setting up the DMA channel.
    SET_WORD(dmaChannel.SRCADDRH, dmaChannel.SRCADDRL, &revbuf);          // The start address of the data to be transmitted
    SET_WORD(dmaChannel.DESTADDRH, dmaChannel.DESTADDRL, HAL_DMA_U0DBUF); // The start address of the destination.
    SET_WORD(dmaChannel.LENH, dmaChannel.LENL, sizeof(revbuf));           // Setting the number of bytes to transfer.
    dmaChannel.VLEN = VLEN_USE_LEN;                                       // Using the length field to determine how many bytes to transfer.
    dmaChannel.PRIORITY = PRI_HIGH;                                       // High priority.
    dmaChannel.M8 = M8_USE_8_BITS;                                        // Irrelevant since length is determined by the LENH and LENL.
    dmaChannel.IRQMASK = FALSE;                                           // The DMA shall not issue an IRQ upon completion.
    dmaChannel.DESTINC = DESTINC_0;                                       // The destination address is to be incremented by 1 after each transfer.
    dmaChannel.SRCINC = SRCINC_1;                                         // The source address inremented by 1 byte after each transfer.
    dmaChannel.TRIG = DMATRIG_URX0;                                       // URX0 ���ܺ󴥷��ж�
    dmaChannel.TMODE = TMODE_BLOCK;                                       // The number of bytes specified by LENH and LENL is transferred.
    dmaChannel.WORDSIZE = WORDSIZE_BYTE;                                  // One byte is transferred each time.
    asm("nop");
    while (1)
    {
        while(!(DMAIRQ & DMA_CHANNEL_0)); //�ȴ�������ϱ�־��λ
    }
}
