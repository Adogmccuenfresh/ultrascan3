#ifndef US_EQUILAPPROACH_H
#define US_EQUILAPPROACH_H
#include "us_plot.h"
#include <QWidget>
#include "us_widgets.h"
#include "us_dataIO.h"
#include <QMouseEvent>
#include<QRubberBand>


class US_EquilApproach : public US_Widgets
{
    Q_OBJECT
public:
    explicit US_EquilApproach(QWidget *parent = nullptr);



private:

    class EQ_Model {
    public:
        QVector<double> time;
        QVector<QVector<double>> yval;
        QVector<double> minreg;
        QVector<double> maxreg;

        void clear(){
            time.clear();
            yval.clear();
            minreg.clear();
            maxreg.clear();

        }


    };



    QPushButton* pb_load;
    QPushButton* pb_reset;
    QLineEdit* le_runID;
    QLineEdit* le_omega;
    QPushButton* pb_next;
    QPushButton* pb_prev;
    QPushButton* pb_startpick;
    QPushButton* pb_endpick;
    QComboBox* cb_lambda;
    QLineEdit* le_start;
    QLineEdit* le_end;





    US_Plot* usplot_up;
    US_Plot* usplot_down;
    QwtPlot* plot_up;
    QwtPlot* plot_down;
    QwtPlotGrid* grid;

    US_PlotPicker *picker;


    QVector<US_DataIO::RawData> rawdata;

    QVector<EQ_Model> rawdata_model;

    US_DataIO::RawData current_rdata;

    QHash<QString, QHash<QString, QVector<int>>> cc_organizer;

    QVector<int> current_idx;
    QVector<int> current_lmbd;
    QVector<double> picked_points;


    QListWidget* lw_triple;



    void ls_triple();
    void plot_rdata();

    void proccess_data();





private slots:
    void load_data();
    void cc_changed(int);
    void lmbd_changed();
    void str_pick();
    void end_pick();
    void mouse_clk(const QwtDoublePoint&);



};

#endif // US_EQUILAPPROACH_H;
