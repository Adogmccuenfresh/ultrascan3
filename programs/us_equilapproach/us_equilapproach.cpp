#include "us_equilapproach.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_images.h"

US_EquilApproach::US_EquilApproach(QWidget *parent)
    : US_Widgets()
{
    // setMinimumSize(800, 400);
    setPalette(US_GuiSettings::frameColor());
    setWindowTitle("Equilibrium Approach");
    pb_load = us_pushbutton("load");
    pb_reset = us_pushbutton("Reset");


    QVBoxLayout* leftlayout = new QVBoxLayout();
    QVBoxLayout* rightlayout = new QVBoxLayout();
    QHBoxLayout* mainlayout = new QHBoxLayout();
    mainlayout->addLayout(leftlayout, 1);
    mainlayout->addLayout(rightlayout, 2);
    setLayout(mainlayout);
    leftlayout->setSpacing(1);
    rightlayout->setSpacing(1);
    mainlayout->setSpacing(1);
    leftlayout->setMargin(0);
    rightlayout->setMargin(0);
    mainlayout->setMargin(1);

    QHBoxLayout* loadlayout = new QHBoxLayout();
    loadlayout->addWidget(pb_load);
    loadlayout->addWidget(pb_reset);

    leftlayout->addLayout(loadlayout);



    QLabel* lb_runID = us_label("RunID");
    le_runID = us_lineedit("",-1, true);

    QHBoxLayout* runIDlayout = new QHBoxLayout();
    runIDlayout->addWidget(lb_runID);
    runIDlayout->addWidget(le_runID);
    leftlayout->addLayout(runIDlayout);
    lw_triple = us_listwidget();
    leftlayout->addWidget(lw_triple);

    static QChar clambda (955); //lambda character

    QLabel* lb_start = us_label(tr("%1 Start").arg(clambda));
    QLabel* lb_end = us_label(tr("%1 End").arg(clambda));

    le_start = us_lineedit("", -1, true);
    le_end = us_lineedit("", -1, true);

    pb_next = us_pushbutton("Next");
    pb_prev = us_pushbutton("Previous");
    pb_next->setIcon(US_Images::getIcon(US_Images::ARROW_RIGHT));
    pb_prev->setIcon(US_Images::getIcon(US_Images::ARROW_LEFT));
    cb_lambda = us_comboBox();





    QGridLayout* lyt_grid = new QGridLayout();
    lyt_grid->addWidget(lb_start,     0, 0, 1, 1);
    lyt_grid->addWidget(le_start,     0, 1, 1, 1);
    lyt_grid->addWidget(cb_lambda,     0, 2, 1, 2);
    lyt_grid->addWidget(lb_end,     0, 4, 1, 1);
    lyt_grid->addWidget(le_end,     0, 5, 1, 1);
    lyt_grid->addWidget(pb_prev,     1, 1, 1, 2);
    lyt_grid->addWidget(pb_next,     1, 3, 1, 2);
    lyt_grid->setSpacing(1);
    leftlayout->addLayout(lyt_grid);

    for (int i = 0; i < 6; i++){
        lyt_grid->setColumnStretch(i,0);
    }


    pb_startpick = us_pushbutton("Beginging Radial Region");
    pb_endpick = us_pushbutton("End Radial Region");
    pb_endpick->setDisabled(true);


    QHBoxLayout* picker_layout = new QHBoxLayout;
    picker_layout->addWidget(pb_startpick);
    picker_layout->addWidget(pb_endpick);


    leftlayout->addLayout(picker_layout);



    // ploting

    //plot_up = new QwtPlot();
    usplot_up = new US_Plot(plot_up, "", "Radius (in cm)", "Intensity");
    usplot_down = new US_Plot(plot_down, "", "Radius (in cm)", "Intensity");

    picker = new US_PlotPicker( plot_up);


    rightlayout->addLayout(usplot_up);
    rightlayout->addLayout(usplot_down);
    leftlayout->addStretch(1);


    // connecting to Qpushbutton for clicking widget
    connect(pb_load, &QPushButton::clicked, this, &US_EquilApproach::load_data);
    connect(pb_startpick, &QPushButton::clicked, this, &US_EquilApproach::str_pick);
    connect(pb_endpick,&QPushButton::clicked, this, &US_EquilApproach::end_pick);







}
void US_EquilApproach::end_pick(){
    if ( picked_points.size() %2 == 1){
        int status;
        status = QMessageBox::question(this, "Warning", "End region has not been defined, to continue"
             " selecting click yes");
        if (status == QMessageBox::Yes){
            return;
        }

        picked_points.clear();
    }
    picker->disconnect();
    pb_endpick->setDisabled(true);
    pb_startpick->setEnabled(true);
    if (picked_points.isEmpty()){
        return;
    }

    proccess_data();








}

void US_EquilApproach::mouse_clk(const QwtDoublePoint& point){
    double x = point.x();
    picked_points << x;
    qDebug() << x;



}

void US_EquilApproach::str_pick(){
    picker->disconnect();
    connect(picker, &US_PlotPicker::cMouseUp, this, &US_EquilApproach::mouse_clk);
    pb_startpick->setDisabled(true);
    pb_endpick->setEnabled(true);

    picked_points.clear();


}

void US_EquilApproach::load_data() {
    QString auc_path = QFileDialog::getExistingDirectory(this, "open auc directory",
                                                         US_Settings::importDir());
    if(auc_path.isEmpty()){
        return;
    }
    QDir dir (auc_path);
    dir.makeAbsolute();
    QStringList name_filter;
    name_filter << "*.auc";
    QFileInfoList auc_finfo_list = dir.entryInfoList(name_filter, QDir::Files, QDir::Name);
    if(auc_finfo_list.isEmpty()){
        return;
    }
    rawdata.clear();
    rawdata_model.clear();

    QString runID;

    for(int ii = 0; ii < auc_finfo_list.size(); ii++){
        QString fpath = auc_finfo_list.at(ii).filePath();
        QString fname = auc_finfo_list.at(ii).baseName();
        qDebug() << fpath;
        US_DataIO::RawData rdata;
        int state = US_DataIO::readRawData(fpath, rdata);
        if (state == US_DataIO::OK){

            if ( runID.isEmpty()){
                runID = fname.section('.', 0, 1);
                rawdata << rdata;
                EQ_Model mdata;
                rawdata_model << mdata;
            } else {
                QString runid = fname.section('.', 0, 1);
                if (runID.compare(runid) == 0){
                    rawdata << rdata;
                    EQ_Model mdata;
                    rawdata_model << mdata;
                }
            }
        }
    }

    qDebug() << rawdata.size();
    if (rawdata.isEmpty() ) {
        QMessageBox::warning(this, "Error!", "No AUC data");
        return;
    }

    le_runID->setText(runID);
    ls_triple();

}

void US_EquilApproach::ls_triple(){
    cc_organizer.clear();
    lw_triple->disconnect();
    lw_triple->clear();

    for (int i = 0; i < rawdata.size(); i++ ){
        QString cell_channel = tr("%1 / %2").arg(rawdata.at(i).cell).arg(rawdata.at(i).channel);
        int wavl = rawdata.at(i).scanData.first().wavelength;
        if( cc_organizer.contains(cell_channel)){
            cc_organizer[cell_channel]["Index"] << i;
            cc_organizer[cell_channel]["Lambda"] << wavl;
        }
        else {
            QHash<QString, QVector<int>> idx_lmbd;
            QVector<int> idx;
            QVector<int> lmbd;
            idx << i;
            lmbd << wavl;
            idx_lmbd["Index"] = idx;
            idx_lmbd["Lambda"] = lmbd;
            cc_organizer.insert(cell_channel,idx_lmbd);
        }


    }

    QHashIterator<QString, QHash<QString, QVector<int>>> it(cc_organizer);
    QStringList cc_list;
    while (it.hasNext()){
        it.next();
        cc_list << it.key();
    }
    cc_list.sort();
    for (int i = 0; i < cc_list.size(); i++){
        lw_triple->addItem(cc_list.at(i));

    }


    connect(lw_triple, &QListWidget::currentRowChanged, this, &US_EquilApproach::cc_changed);
    lw_triple->setCurrentRow(0);




}

void US_EquilApproach::cc_changed(int row){
    current_idx.clear();
    current_lmbd.clear();
    QString key = lw_triple->item(row)->text();
    current_idx << cc_organizer.value(key).value("Index");
    current_lmbd << cc_organizer.value(key).value("Lambda");
    le_start->setText(QString::number(current_lmbd.first()));
    le_end->setText(QString::number(current_lmbd.last()));

    cb_lambda->disconnect();
    cb_lambda->clear();
    for (int i = 0; i < current_lmbd.size(); i++){
        QString item = QString::number(current_lmbd.at(i));
        cb_lambda->addItem(item);
    }


    connect(cb_lambda, &QComboBox::currentTextChanged,
            this, &US_EquilApproach::lmbd_changed);

    cb_lambda->setCurrentIndex(0);
    lmbd_changed();






}

void US_EquilApproach::lmbd_changed(){
    int lambda_idx = cb_lambda->currentIndex();
    int rawdata_idx = current_idx.at(lambda_idx);
    current_rdata = rawdata.at(rawdata_idx);
    plot_rdata();




}

void US_EquilApproach::plot_rdata(){
    plot_up->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    int n_scan = current_rdata.scanCount();
    int n_points = current_rdata.pointCount();
    const double* xp = current_rdata.xvalues.data();
    const double* yp;

    for (int i = 0; i < n_scan; i++){
        QString label = tr("Scan %1").arg(i+1);
        QwtPlotCurve* curve = us_curve( plot_up, label );
        QPen pen(Qt::yellow);
        curve->setPen(pen);
        yp = current_rdata.scanData.at(i).rvalues.data();
        curve->setSamples(xp, yp, n_points);


    }


    plot_up->replot();




}

void US_EquilApproach::proccess_data(){

}

