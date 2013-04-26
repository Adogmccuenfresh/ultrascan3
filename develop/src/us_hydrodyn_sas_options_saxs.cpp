#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsSaxs::US_Hydrodyn_SasOptionsSaxs(struct saxs_options *saxs_options, bool *sas_options_saxs_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_saxs_widget = sas_options_saxs_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_saxs_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SAXS Computation Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsSaxs::~US_Hydrodyn_SasOptionsSaxs()
{
   *sas_options_saxs_widget = false;
}

void US_Hydrodyn_SasOptionsSaxs::setupGUI()
{
   started_in_expert_mode = ((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode;

   int minHeight1 = 30;
   lbl_info = new QLabel(tr("US-SOMO SAXS Computation Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_water_e_density = new QLabel(tr(" Water electron density (e / A^3): "), this);
   lbl_water_e_density->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_water_e_density->setMinimumHeight(minHeight1);
   lbl_water_e_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_water_e_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_water_e_density = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_water_e_density );
   cnt_water_e_density->setRange(0, 10, 0.001);
   cnt_water_e_density->setValue((*saxs_options).water_e_density);
   cnt_water_e_density->setMinimumHeight(minHeight1);
   cnt_water_e_density->setEnabled(true);
   cnt_water_e_density->setNumButtons(3);
   cnt_water_e_density->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_water_e_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_water_e_density, SIGNAL(valueChanged(double)), SLOT(update_water_e_density(double)));

   lbl_saxs_iq = new QLabel(tr(" I(q) method: "), this);
   lbl_saxs_iq->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_iq->setMinimumHeight(minHeight1);
   lbl_saxs_iq->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_saxs_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_saxs_iq_native_debye = new QCheckBox(this);
   cb_saxs_iq_native_debye->setText(tr("F-DB "));
   cb_saxs_iq_native_debye->setEnabled(true);
   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_debye->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_debye, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_debye()));

   cb_saxs_iq_native_sh = new QCheckBox(this);
   cb_saxs_iq_native_sh->setText(tr("SH-DB "));
   cb_saxs_iq_native_sh->setEnabled(true);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   cb_saxs_iq_native_sh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_sh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_sh, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_sh()));

   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid = new QCheckBox(this);
      cb_saxs_iq_native_hybrid->setText(tr("Hybrid "));
      cb_saxs_iq_native_hybrid->setEnabled(true);
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_saxs_iq_native_hybrid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_saxs_iq_native_hybrid, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_hybrid()));

      cb_saxs_iq_native_hybrid2 = new QCheckBox(this);
      cb_saxs_iq_native_hybrid2->setText(tr("H2 "));
      cb_saxs_iq_native_hybrid2->setEnabled(true);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_saxs_iq_native_hybrid2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_saxs_iq_native_hybrid2, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_hybrid2()));

      cb_saxs_iq_native_hybrid3 = new QCheckBox(this);
      cb_saxs_iq_native_hybrid3->setText(tr("H3 "));
      cb_saxs_iq_native_hybrid3->setEnabled(true);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
      cb_saxs_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_saxs_iq_native_hybrid3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_saxs_iq_native_hybrid3, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_hybrid3()));

      cb_saxs_iq_hybrid_adaptive = new QCheckBox(this);
      cb_saxs_iq_hybrid_adaptive->setText(tr("Adaptive "));
      cb_saxs_iq_hybrid_adaptive->setEnabled(true);
      cb_saxs_iq_hybrid_adaptive->setChecked((*saxs_options).saxs_iq_hybrid_adaptive);
      cb_saxs_iq_hybrid_adaptive->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_saxs_iq_hybrid_adaptive->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_saxs_iq_hybrid_adaptive, SIGNAL(clicked()), this, SLOT(set_saxs_iq_hybrid_adaptive()));
   }

   cb_saxs_iq_native_fast = new QCheckBox(this);
   cb_saxs_iq_native_fast->setText(tr("Q-DB "));
   cb_saxs_iq_native_fast->setEnabled(true);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_fast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_fast, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_fast()));

   cb_saxs_iq_native_fast_compute_pr = new QCheckBox(this);
   cb_saxs_iq_native_fast_compute_pr->setText(tr("P(r) "));
   cb_saxs_iq_native_fast_compute_pr->setEnabled(true);
   cb_saxs_iq_native_fast_compute_pr->setChecked((*saxs_options).saxs_iq_native_fast_compute_pr);
   cb_saxs_iq_native_fast_compute_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_fast_compute_pr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_fast_compute_pr, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_fast_compute_pr()));

   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs = new QCheckBox(this);
      cb_saxs_iq_foxs->setText(tr("FoXS"));
      cb_saxs_iq_foxs->setEnabled(true);
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_foxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_saxs_iq_foxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_saxs_iq_foxs, SIGNAL(clicked()), this, SLOT(set_saxs_iq_foxs()));
   }

   cb_saxs_iq_crysol = new QCheckBox(this);
   cb_saxs_iq_crysol->setText(tr("Crysol"));
   cb_saxs_iq_crysol->setEnabled(true);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_crysol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_crysol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_crysol, SIGNAL(clicked()), this, SLOT(set_saxs_iq_crysol()));

   if ( started_in_expert_mode )
   {
      cb_saxs_iq_sastbx = new QCheckBox(this);
      cb_saxs_iq_sastbx->setText(tr("Sastbx"));
      cb_saxs_iq_sastbx->setEnabled(true);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
      cb_saxs_iq_sastbx->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_saxs_iq_sastbx->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_saxs_iq_sastbx, SIGNAL(clicked()), this, SLOT(set_saxs_iq_sastbx()));
   }

   lbl_fast_bin_size = new QLabel(tr(" Quick Debye: Bin size"), this);
   lbl_fast_bin_size->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_fast_bin_size->setMinimumHeight(minHeight1);
   lbl_fast_bin_size->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_fast_bin_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_fast_bin_size = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_fast_bin_size );
   cnt_fast_bin_size->setRange(0.01, 100, 0.01);
   cnt_fast_bin_size->setValue((*saxs_options).fast_bin_size);
   cnt_fast_bin_size->setMinimumHeight(minHeight1);
   cnt_fast_bin_size->setEnabled(true);
   cnt_fast_bin_size->setNumButtons(2);
   cnt_fast_bin_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_fast_bin_size->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_fast_bin_size, SIGNAL(valueChanged(double)), SLOT(update_fast_bin_size(double)));

   lbl_fast_modulation = new QLabel(tr(" Quick Debye: Modulation"), this);
   lbl_fast_modulation->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_fast_modulation->setMinimumHeight(minHeight1);
   lbl_fast_modulation->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_fast_modulation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_fast_modulation = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_fast_modulation );
   cnt_fast_modulation->setRange(0.1, 0.5, 0.001);
   cnt_fast_modulation->setValue((*saxs_options).fast_modulation);
   cnt_fast_modulation->setMinimumHeight(minHeight1);
   cnt_fast_modulation->setEnabled(true);
   cnt_fast_modulation->setNumButtons(3);
   cnt_fast_modulation->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_fast_modulation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_fast_modulation, SIGNAL(valueChanged(double)), SLOT(update_fast_modulation(double)));

   lbl_hybrid2_q_points = new QLabel(tr(" Hybrid, H2, H3: q points"), this);
   lbl_hybrid2_q_points->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid2_q_points->setMinimumHeight(minHeight1);
   lbl_hybrid2_q_points->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_hybrid2_q_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_hybrid2_q_points = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_hybrid2_q_points );
   cnt_hybrid2_q_points->setRange(3, 100, 1);
   cnt_hybrid2_q_points->setValue((*saxs_options).hybrid2_q_points);
   cnt_hybrid2_q_points->setMinimumHeight(minHeight1);
   cnt_hybrid2_q_points->setEnabled(true);
   cnt_hybrid2_q_points->setNumButtons(3);
   cnt_hybrid2_q_points->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_hybrid2_q_points->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_hybrid2_q_points, SIGNAL(valueChanged(double)), SLOT(update_hybrid2_q_points(double)));

   lbl_sh_max_harmonics = new QLabel( started_in_expert_mode ?
                                      tr(" SH/Crysol/Sastbx: Maximum order of harmonics")
                                      :
                                      tr(" SH/Crysol: Maximum order of harmonics")
                                      , this);
   lbl_sh_max_harmonics->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sh_max_harmonics->setMinimumHeight(minHeight1);
   lbl_sh_max_harmonics->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sh_max_harmonics->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_sh_max_harmonics = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_sh_max_harmonics );
   cnt_sh_max_harmonics->setRange(1, 75, 1);
   cnt_sh_max_harmonics->setValue((*saxs_options).sh_max_harmonics);
   cnt_sh_max_harmonics->setMinimumHeight(minHeight1);
   cnt_sh_max_harmonics->setEnabled(true);
   cnt_sh_max_harmonics->setNumButtons(2);
   cnt_sh_max_harmonics->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_sh_max_harmonics->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_sh_max_harmonics, SIGNAL(valueChanged(double)), SLOT(update_sh_max_harmonics(double)));

   lbl_sh_fibonacci_grid_order = new QLabel( started_in_expert_mode ? 
                                             tr(" Crysol/Sastbx: Order of Fibonacci grid")
                                             :
                                             tr(" Crysol: Order of Fibonacci grid")
                                             , this);
   lbl_sh_fibonacci_grid_order->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sh_fibonacci_grid_order->setMinimumHeight(minHeight1);
   lbl_sh_fibonacci_grid_order->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sh_fibonacci_grid_order->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_sh_fibonacci_grid_order = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_sh_fibonacci_grid_order );
   cnt_sh_fibonacci_grid_order->setRange(2, 75, 1);
   cnt_sh_fibonacci_grid_order->setValue((*saxs_options).sh_fibonacci_grid_order);
   cnt_sh_fibonacci_grid_order->setMinimumHeight(minHeight1);
   cnt_sh_fibonacci_grid_order->setEnabled(true);
   cnt_sh_fibonacci_grid_order->setNumButtons(2);
   cnt_sh_fibonacci_grid_order->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_sh_fibonacci_grid_order->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_sh_fibonacci_grid_order, SIGNAL(valueChanged(double)), SLOT(update_sh_fibonacci_grid_order(double)));

   lbl_crysol_hydration_shell_contrast = new QLabel( started_in_expert_mode ?
                                                     tr(" Crysol/Sastbx: Contrast of hydration shell (e / A^3):")
                                                     :
                                                     tr(" Crysol: Contrast of hydration shell (e / A^3):")
                                                     , this);
   lbl_crysol_hydration_shell_contrast->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_crysol_hydration_shell_contrast->setMinimumHeight(minHeight1);
   lbl_crysol_hydration_shell_contrast->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_crysol_hydration_shell_contrast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_crysol_hydration_shell_contrast = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_crysol_hydration_shell_contrast );
   cnt_crysol_hydration_shell_contrast->setRange(-1, 1, 0.001);
   cnt_crysol_hydration_shell_contrast->setValue((*saxs_options).crysol_hydration_shell_contrast);
   cnt_crysol_hydration_shell_contrast->setMinimumHeight(minHeight1);
   cnt_crysol_hydration_shell_contrast->setEnabled(true);
   cnt_crysol_hydration_shell_contrast->setNumButtons(3);
   cnt_crysol_hydration_shell_contrast->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_crysol_hydration_shell_contrast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_crysol_hydration_shell_contrast, SIGNAL(valueChanged(double)), SLOT(update_crysol_hydration_shell_contrast(double)));

   lbl_crysol = new QLabel(tr(" Crysol options:"), this);
   lbl_crysol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_crysol->setMinimumHeight(minHeight1);
   lbl_crysol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_crysol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_crysol_default_load_difference_intensity = new QCheckBox(this);
   cb_crysol_default_load_difference_intensity->setText(tr(" Automatically load difference intensity"));
   cb_crysol_default_load_difference_intensity->setEnabled(true);
   cb_crysol_default_load_difference_intensity->setChecked((*saxs_options).crysol_default_load_difference_intensity);
   cb_crysol_default_load_difference_intensity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_crysol_default_load_difference_intensity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_crysol_default_load_difference_intensity, SIGNAL(clicked()), this, SLOT(set_crysol_default_load_difference_intensity()));

   cb_crysol_version_26 = new QCheckBox(this);
   cb_crysol_version_26->setText(tr(" Support version 2.6"));
   cb_crysol_version_26->setEnabled(true);
   cb_crysol_version_26->setChecked((*saxs_options).crysol_version_26);
   cb_crysol_version_26->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_crysol_version_26->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_crysol_version_26, SIGNAL(clicked()), this, SLOT(set_crysol_version_26()));

   cb_crysol_explicit_hydrogens = new QCheckBox(this);
   cb_crysol_explicit_hydrogens->setText(tr(" Explicit hydrogens"));
   cb_crysol_explicit_hydrogens->setEnabled(true);
   cb_crysol_explicit_hydrogens->setChecked((*saxs_options).crysol_explicit_hydrogens);
   cb_crysol_explicit_hydrogens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_crysol_explicit_hydrogens->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_crysol_explicit_hydrogens, SIGNAL(clicked()), this, SLOT(set_crysol_explicit_hydrogens()));

   if ( started_in_expert_mode )
   {
      lbl_sastbx_method = new QLabel(tr(" Sastbx: Method"), this);
      lbl_sastbx_method->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
      lbl_sastbx_method->setMinimumHeight(minHeight1);
      lbl_sastbx_method->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
      lbl_sastbx_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

      rb_sastbx_method_she = new QRadioButton(tr("Spherical harmonics"), this);
      rb_sastbx_method_she->setEnabled(true);
      rb_sastbx_method_she->setChecked( saxs_options->sastbx_method == 0 );
      rb_sastbx_method_she->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sastbx_method_she->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

      rb_sastbx_method_debye = new QRadioButton(tr("Debye"), this);
      rb_sastbx_method_debye->setEnabled(true);
      rb_sastbx_method_debye->setChecked( saxs_options->sastbx_method == 1 );
      rb_sastbx_method_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sastbx_method_debye->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

      rb_sastbx_method_zernike = new QRadioButton(tr("Zernike"), this);
      rb_sastbx_method_zernike->setEnabled(true);
      rb_sastbx_method_zernike->setChecked( saxs_options->sastbx_method == 2 );
      rb_sastbx_method_zernike->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      rb_sastbx_method_zernike->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

      bg_sastbx_method = new QButtonGroup(1, Qt::Horizontal, 0);
      bg_sastbx_method->setRadioButtonExclusive(true);
      bg_sastbx_method->insert(rb_sastbx_method_she);
      bg_sastbx_method->insert(rb_sastbx_method_debye);
      bg_sastbx_method->insert(rb_sastbx_method_zernike);

      connect(bg_sastbx_method, SIGNAL(clicked(int)), SLOT(set_sastbx_method(int)));
   }

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   background->addWidget(lbl_water_e_density, j, 0);
   background->addWidget(cnt_water_e_density, j, 1);
   j++;

   QHBoxLayout *hbl_saxs_iq = new QHBoxLayout;
   hbl_saxs_iq->addWidget(lbl_saxs_iq);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_debye);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_sh);

   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(cb_saxs_iq_native_hybrid);
      hbl_saxs_iq->addWidget(cb_saxs_iq_native_hybrid2);
      hbl_saxs_iq->addWidget(cb_saxs_iq_native_hybrid3);
      hbl_saxs_iq->addWidget(cb_saxs_iq_hybrid_adaptive);
   }
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_fast);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_fast_compute_pr);
   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(cb_saxs_iq_foxs);
   }
   hbl_saxs_iq->addWidget(cb_saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      hbl_saxs_iq->addWidget(cb_saxs_iq_sastbx);
   }
   background->addMultiCellLayout(hbl_saxs_iq, j, j, 0, 1);
   j++;

   background->addWidget(lbl_fast_bin_size, j, 0);
   background->addWidget(cnt_fast_bin_size, j, 1);
   j++;

   background->addWidget(lbl_fast_modulation, j, 0);
   background->addWidget(cnt_fast_modulation, j, 1);
   j++;

   background->addWidget(lbl_hybrid2_q_points, j, 0);
   background->addWidget(cnt_hybrid2_q_points, j, 1);
   j++;

   background->addWidget(lbl_sh_max_harmonics, j, 0);
   background->addWidget(cnt_sh_max_harmonics, j, 1);
   j++;

   background->addWidget(lbl_sh_fibonacci_grid_order, j, 0);
   background->addWidget(cnt_sh_fibonacci_grid_order, j, 1);
   j++;

   background->addWidget(lbl_crysol_hydration_shell_contrast, j, 0);
   background->addWidget(cnt_crysol_hydration_shell_contrast, j, 1);
   j++;

   QHBoxLayout *hbl_crysol = new QHBoxLayout;
   hbl_crysol->addWidget(lbl_crysol);
   hbl_crysol->addWidget(cb_crysol_default_load_difference_intensity);
   hbl_crysol->addWidget(cb_crysol_version_26);
   hbl_crysol->addWidget(cb_crysol_explicit_hydrogens);
   background->addMultiCellLayout(hbl_crysol, j, j, 0, 1);
   j++;

   if ( started_in_expert_mode )
   {
      background->addWidget(lbl_sastbx_method, j, 0);
      QHBoxLayout *hbl_sastbx_method = new QHBoxLayout;
      // hbl_sastbx_method->addWidget( bg_sastbx_method );
      hbl_sastbx_method->addWidget( rb_sastbx_method_she );
      hbl_sastbx_method->addWidget( rb_sastbx_method_debye );
      hbl_sastbx_method->addWidget( rb_sastbx_method_zernike );
      background->addLayout( hbl_sastbx_method, j, 1 );
      j++;
   }

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );
   //    if ( !started_in_expert_mode )
   //    {
   //       cb_saxs_iq_native_hybrid->hide();
   //       cb_saxs_iq_native_hybrid2->hide();
   //       cb_saxs_iq_native_hybrid3->hide();
   //       cb_saxs_iq_hybrid_adaptive->hide();
   //       cb_saxs_iq_foxs->hide();
   //       cb_saxs_iq_sastbx->hide();
   //    }      
   if ( !started_in_expert_mode )
   {
      lbl_hybrid2_q_points->hide();
      cnt_hybrid2_q_points->hide();
   }
}

void US_Hydrodyn_SasOptionsSaxs::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsSaxs::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   // online_help->show_help("manual/somo_sas_options_saxs.html");
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsSaxs::closeEvent(QCloseEvent *e)
{
   *sas_options_saxs_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsSaxs::update_water_e_density(double val)
{
   (*saxs_options).water_e_density = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_debye()
{
   (*saxs_options).saxs_iq_native_debye = cb_saxs_iq_native_debye->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_fast = !cb_saxs_iq_native_debye->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_sh()
{
   (*saxs_options).saxs_iq_native_sh = cb_saxs_iq_native_sh->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_sh->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_hybrid()
{
   (*saxs_options).saxs_iq_native_hybrid = cb_saxs_iq_native_hybrid->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_hybrid->isChecked();
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_hybrid2()
{
   (*saxs_options).saxs_iq_native_hybrid2 = cb_saxs_iq_native_hybrid2->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_hybrid2->isChecked();
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_hybrid3()
{
   (*saxs_options).saxs_iq_native_hybrid3 = cb_saxs_iq_native_hybrid3->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_hybrid3->isChecked();
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_hybrid_adaptive()
{
   (*saxs_options).saxs_iq_hybrid_adaptive = cb_saxs_iq_hybrid_adaptive->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_fast()
{
   (*saxs_options).saxs_iq_native_fast = cb_saxs_iq_native_fast->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_fast->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_native_fast_compute_pr()
{
   (*saxs_options).saxs_iq_native_fast_compute_pr = cb_saxs_iq_native_fast_compute_pr->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_crysol()
{
   (*saxs_options).saxs_iq_crysol = cb_saxs_iq_crysol->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_crysol->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_foxs()
{
   (*saxs_options).saxs_iq_foxs = cb_saxs_iq_foxs->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_foxs->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_sastbx = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
      cb_saxs_iq_sastbx->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_saxs_iq_sastbx()
{
   (*saxs_options).saxs_iq_sastbx = cb_saxs_iq_sastbx->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_foxs->isChecked();
   (*saxs_options).saxs_iq_native_sh = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_crysol = false;
   (*saxs_options).saxs_iq_foxs   = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_sh->setChecked((*saxs_options).saxs_iq_native_sh);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
      cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
      cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   }
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   if ( started_in_expert_mode )
   {
      cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_sastbx);
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSaxs::update_sh_max_harmonics(double val)
{
   (*saxs_options).sh_max_harmonics = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SasOptionsSaxs::update_sh_fibonacci_grid_order(double val)
{
   (*saxs_options).sh_fibonacci_grid_order = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SasOptionsSaxs::update_crysol_hydration_shell_contrast(double val)
{
   (*saxs_options).crysol_hydration_shell_contrast = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SasOptionsSaxs::set_crysol_default_load_difference_intensity()
{
   (*saxs_options).crysol_default_load_difference_intensity = cb_crysol_default_load_difference_intensity->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::set_crysol_version_26()
{
   (*saxs_options).crysol_version_26 = cb_crysol_version_26->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::set_crysol_explicit_hydrogens()
{
   (*saxs_options).crysol_explicit_hydrogens = cb_crysol_explicit_hydrogens->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::update_fast_modulation(double val)
{
   (*saxs_options).fast_modulation = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::update_hybrid2_q_points(double val)
{
   (*saxs_options).hybrid2_q_points = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::update_fast_bin_size(double val)
{
   (*saxs_options).fast_bin_size = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSaxs::set_sastbx_method( int val )
{
   saxs_options->sastbx_method = ( unsigned int ) val;
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
