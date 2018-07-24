package inet_dsme.routing.gpsr;

import inet.routing.gpsr.Gpsr;

simple SP_GPSR extends Gpsr
{
    parameters:
        bool straightPath = default(true);
        @class(::inet_dsme::SP_GPSR);
}
