CC := gcc
OBJS := test.o fri13th.o
CFLAGS := -W -Wall
TARGET := test
MACROS :=
LIBS := -lcunit

ifeq ($(BUILD), debug)
	CFLAGS += -g
	MACROS += -DDEBUG_OUTPUT
endif

ifeq ($(BUILD), release)
	CFLAGS += -O3
endif

ifeq ($(TUNE), native)
	CFLAGS += -march=native
endif

test : $(OBJS)

$(OBJS): %.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(MACROS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean $(TARGET)