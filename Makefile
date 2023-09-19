TARGET = pp2dat

SRCS := $(shell find ./src -name *.c)
OBJS := $(addsuffix .o,$(basename $(SRCS)))

test: $(TARGET)
	@./test/test.sh

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.PHONY: test clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)


