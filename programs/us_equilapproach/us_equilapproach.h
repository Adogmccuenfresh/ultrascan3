#ifndef US_EQUILAPPROACH_H
#define US_EQUILAPPROACH_H
#include "us_plot.h"
#include <QWidget>
#include "us_widgets.h"
#include "us_dataIO.h"
#include <QMouseEvent>



class US_EquilApproach : public US_Widgets
{
    Q_OBJECT
public:
    explicit US_EquilApproach(QWidget *parent = nullptr);



private:

    class EQ_Model {
    public:
        QVector<double> time;
        QVector<QVector<double>> yval; // multiple regions, hence vector of a vector
        QVector<double> minreg;
        QVector<double> maxreg;
        // Qvector for nnls fitting holds the parameters of C, A, B for every species
        //  Y = Sigma(C* (A/(1+e^Bx))), sum over all sigmoids
        QVector<QVector<double>> model;




        void clear(){
            time.clear();
            yval.clear();
            minreg.clear();
            maxreg.clear();

        }


    };



    QPushButton* pb_load;
    QPushButton* pb_reset;
    QPushButton* pb_next;
    QPushButton* pb_prev;
    QPushButton* pb_startpick;
    QPushButton* pb_endpick;
    QPushButton* pb_save;
    QPushButton* pb_fit;

    QLineEdit* le_runID;
    QLineEdit* le_omega;
    QLineEdit* le_start;
    QLineEdit* le_end;
    QLineEdit* le_ngrids;
    QLineEdit* le_nrange;

    QComboBox* cb_lambda;





    US_Plot* usplot_up;
    US_Plot* usplot_down;
    QwtPlot* plot_up;
    QwtPlot* plot_down;
    QwtPlotGrid* grid;


    US_PlotPicker *picker;


    QVector<US_DataIO::RawData> rawdata;

    QList<EQ_Model> rawdata_model;

    US_DataIO::RawData current_rdata;
    EQ_Model current_mdata;

    QHash<QString, QHash<QString, QVector<int>>> cc_organizer;

    QVector<int> current_idx;
    QVector<int> current_lmbd;
    QVector<double> picked_points;


    QListWidget* lw_triple;



    void ls_triple();
    void plot_rdata();
    void plot_mdata();
    void proccess_data();
    void proccess_fit(EQ_Model&);





private slots:
    void load_data();
    void cc_changed(int);
    void lmbd_changed();
    void str_pick();
    void end_pick();
    void mouse_clk(const QwtDoublePoint&);

    // save the model plot for nnls
    void save_mplot();

    void strt_fit();


};

#endif // US_EQUILAPPROACH_H;
