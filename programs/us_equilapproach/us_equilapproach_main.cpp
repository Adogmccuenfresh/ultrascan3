#include "us_equilapproach.h"
#include <QApplication>
#include "us_dataIO.h"
#include "us_license.h"
#include "us_license_t.h"

 int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    #include "main1.inc"
    US_EquilApproach * w = new US_EquilApproach();
    w->show();

    return application.exec();



}
