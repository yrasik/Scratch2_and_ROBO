#ifndef FIFO__H
#define FIFO__H
 
//размер должен быть степенью двойки: 4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768. !!! нельзя 65536 и выше
//Заточен под uint8_t
#define FIFO( size )\
  struct {\
    uint16_t buf[size];\
    uint16_t tail;\
    uint16_t head;\
  } 
 
//количество элементов в очереди
#define FIFO_COUNT(fifo)     (fifo.head-fifo.tail)
 
//размер fifo
#define FIFO_SIZE(fifo)      ( sizeof(fifo.buf)/sizeof(fifo.buf[0]) )
 
//fifo заполнено?
#define FIFO_IS_FULL(fifo)   (FIFO_COUNT(fifo)==FIFO_SIZE(fifo))
 
//fifo пусто?
#define FIFO_IS_EMPTY(fifo)  (fifo.tail==fifo.head)
 
//количество свободного места в fifo
#define FIFO_SPACE(fifo)     (FIFO_SIZE(fifo)-FIFO_COUNT(fifo))
 
//поместить элемент в fifo
#define FIFO_PUSH(fifo, byte) \
  {\
    fifo.buf[fifo.head & (FIFO_SIZE(fifo)-1)]=byte;\
    fifo.head++;\
  }
 
//взять первый элемент из fifo
#define FIFO_FRONT(fifo) (fifo.buf[fifo.tail & (FIFO_SIZE(fifo)-1)])
 
//уменьшить количество элементов в очереди
#define FIFO_POP(fifo)   \
  {\
      fifo.tail++; \
  }
 
//очистить fifo
#define FIFO_FLUSH(fifo)   \
  {\
    fifo.tail=0;\
    fifo.head=0;\
  } 
 
#endif //FIFO__H
