# initial
FROM debian:buster
MAINTAINER am418402

# installing required packages
RUN apt-get -y update
RUN apt-get -y install build-essential
RUN apt-get -y install openssh-server
RUN apt-get -y install sudo
RUN apt-get -y install acl
RUN apt-get -y install libpam0g-dev
RUN apt-get -y install python3
RUN apt-get -y install python3-pip
RUN pip3 install django

# setting up users
RUN groupadd officers
RUN groupadd clients
ADD user_setup.sh user_setup.sh
ADD uzytkownicy.txt uzytkownicy.txt
RUN chmod a+x user_setup.sh
RUN bash ./user_setup.sh

# adding the employee app
ADD gfb_app_src/bank_app.c bank_app.c
ADD gfb_app_src/Makefile Makefile
ADD gfb_app_src/pam_gfb.h pam_gfb.h
ADD gfb_app_src/pam_gfb.c pam_gfb.c
ADD gfb_app_src/input_gfb.h input_gfb.h
ADD gfb_app_src/input_gfb.c input_gfb.c
ADD gfb_app_src/types_gfb.h types_gfb.h
RUN make bank_app

# adding automatic app startup on login
ADD app_start.sh app_start.sh
RUN mv app_start.sh /etc/profile.d

# setting up directories with permissions
RUN mkdir deposits
RUN mkdir credits
ADD permissions_setup.sh permissions_setup.sh
RUN chmod a+x permissions_setup.sh
RUN bash ./permissions_setup.sh

# enabling officers to transfer file ownership to clients (required)
RUN echo "Cmnd_Alias CEDE = /bin/chown" >> /etc/sudoers
RUN echo "%officers  ALL=(ALL:ALL) NOPASSWD:CEDE" >> /etc/sudoers

# adding the client app
ADD client_app client_app
RUN python3 client_app/manage.py create_gfb_users

# adding server certificate
ADD create_cert.sh create_cert.sh
RUN chmod a+x create_cert.sh
RUN bash ./create_cert.sh

# cleaning up
RUN rm uzytkownicy.txt
RUN rm user_setup.sh
RUN rm create_cert.sh

# starting ssh server on an arbitrary port
EXPOSE 22 2137
ENTRYPOINT service ssh start && python3 client_app/manage.py runserver 0.0.0.0:2137
