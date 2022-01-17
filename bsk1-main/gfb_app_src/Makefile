all: bank_app

bank_app: bank_app.c pam_gfb.c input_gfb.c
	gcc -std=c11 -g -o bank_app bank_app.c pam_gfb.c input_gfb.c -lpam -lpam_misc

clean:
	rm bank_app
