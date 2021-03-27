FROM gcc:10

WORKDIR /build
COPY . ./

RUN make drat-trim

RUN : \
&& ./drat-trim examples/uuf-100-1.cnf examples/uuf-100-1.drat -c core1 -l lemmas1 \
&& ./drat-trim core1 lemmas1 \
&& ./drat-trim -c core2 examples/uuf-100-1.cnf -l lemmas2 examples/uuf-100-1.drat \
&& ./drat-trim core2 lemmas2 \
&& ./drat-trim -c core3 examples/uuf-100-1.cnf -l lemmas3 < examples/uuf-100-1.drat \
&& ./drat-trim core3 lemmas3 \
&& cmp core1 core2 \
&& cmp core2 core3 \
&& cmp lemmas1 lemmas2 \
&& cmp lemmas2 lemmas3


