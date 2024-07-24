#include <views/framework.h>

class AppShimStopwatch: public Composite {
public:
    AppShimStopwatch(Beeper *);
    ~AppShimStopwatch();
private:
    class StopwatchMainScreen;
    StopwatchMainScreen * appRoot;
};