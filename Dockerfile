FROM rootproject/root

RUN yum -y install boost

RUN echo ${BOOST_ROOT}
