.PHONY := clean
object = client server
all: $(object)

$(object): %: %.c
		gcc $< -o $@ -lrt


.PHONY := clean
clean:
		rm -rf $(object)


