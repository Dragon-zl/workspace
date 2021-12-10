#app : main.c  printf.c
#	gcc main.c printf.c -o app
src=$(wildcard  ./*.c)
objs=$(patsubst %.c , %.o , $(src))
target=app
$(target) : $(objs)
	$(CC) $(objs) -o $(target)
%.o : %.c
	$(CC) -c $< -o $@
.PHONY:clean		#将clean设置为伪目标
clean:
	rm  $(objs)  -f