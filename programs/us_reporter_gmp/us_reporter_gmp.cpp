#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>

#include "us_reporter_gmp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_protocol_util.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_lamm_astfvm.h"
#include "../us_fematch/us_thread_worker.h"

#define MIN_NTC   25

// Constructor
US_ReporterGMP::US_ReporterGMP() : US_Widgets()
{
  setWindowTitle( tr( "GMP Report Generator"));
  setPalette( US_GuiSettings::frameColor() );
  
  // primary layouts
  QHBoxLayout* mainLayout     = new QHBoxLayout( this );
  QVBoxLayout* leftLayout     = new QVBoxLayout();
  QVBoxLayout* rghtLayout     = new QVBoxLayout();
  QGridLayout* buttonsLayout  = new QGridLayout();
  QGridLayout* genTreeLayout  = new QGridLayout();
  QGridLayout* perChanTreeLayout  = new QGridLayout();
  mainLayout->setSpacing        ( 2 );
  mainLayout->setContentsMargins( 2, 2, 2, 2 );
  leftLayout->setSpacing        ( 0 );
  leftLayout->setContentsMargins( 0, 1, 0, 1 );
  rghtLayout->setSpacing        ( 0 );
  rghtLayout->setContentsMargins( 0, 1, 0, 1 );
  buttonsLayout->setSpacing     ( 1 );
  buttonsLayout->setContentsMargins( 0, 0, 0, 0 );
  genTreeLayout->setSpacing        ( 1 );
  genTreeLayout->setContentsMargins( 0, 0, 0, 0 );
  perChanTreeLayout->setSpacing        ( 1 );
  perChanTreeLayout->setContentsMargins( 0, 0, 0, 0 );

  //leftLayout
  QLabel*      bn_actions     = us_banner( tr( "Actions:" ), 1 );
  QLabel*      lb_loaded_run  = us_label( tr( "Loaded Run:" ) );
  le_loaded_run               = us_lineedit( tr(""), 0, true );

  QPushButton* pb_loadrun       = us_pushbutton( tr( "Load GMP Run" ) );
  pb_gen_report    = us_pushbutton( tr( "Generate Report" ) );
  pb_view_report   = us_pushbutton( tr( "View Report" ) );
  pb_select_all    = us_pushbutton( tr( "Select All" ) );
  pb_unselect_all  = us_pushbutton( tr( "Unselect All" ) );
  pb_expand_all    = us_pushbutton( tr( "Expand All" ) );
  pb_collapse_all  = us_pushbutton( tr( "Collapse All" ) );
  pb_help          = us_pushbutton( tr( "Help" ) );
  pb_close         = us_pushbutton( tr( "Close" ) );
		
  int row           = 0;
  buttonsLayout->addWidget( bn_actions,     row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_loaded_run,  row,   0, 1, 2 );
  buttonsLayout->addWidget( le_loaded_run,  row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_loadrun,     row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_gen_report,  row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_view_report, row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_select_all,  row  , 0, 1, 6 );
  buttonsLayout->addWidget( pb_unselect_all,row++, 6, 1, 6 );
  buttonsLayout->addWidget( pb_expand_all,  row  , 0, 1, 6 );
  buttonsLayout->addWidget( pb_collapse_all,row++, 6, 1, 6 );

  buttonsLayout->addWidget( pb_help,        row,   0, 1, 6, Qt::AlignBottom );
  buttonsLayout->addWidget( pb_close,       row++, 6, 1, 6, Qt::AlignBottom );

  pb_gen_report  ->setEnabled( false );
  pb_view_report ->setEnabled( false );
  pb_select_all  ->setEnabled( false );
  pb_unselect_all->setEnabled( false );
  pb_expand_all  ->setEnabled( false );
  pb_collapse_all->setEnabled( false );
  
  connect( pb_help,    SIGNAL( clicked()      ),
	   this,       SLOT(   help()         ) );
  connect( pb_close,   SIGNAL( clicked()      ),
	   this,       SLOT(   close()        ) );

  connect( pb_loadrun,      SIGNAL( clicked()      ),
	   this,            SLOT(   load_gmp_run()   ) );
  connect( pb_gen_report,   SIGNAL( clicked()      ),
	   this,            SLOT(   generate_report()   ) );
  connect( pb_view_report,  SIGNAL( clicked()      ),
	   this,            SLOT(   view_report()   ) );
  connect( pb_select_all,   SIGNAL( clicked()      ),
	   this,            SLOT( select_all()   ) );
  connect( pb_unselect_all, SIGNAL( clicked()      ),
	   this,            SLOT(   unselect_all()   ) );
  connect( pb_expand_all,   SIGNAL( clicked()      ),
	   this,            SLOT( expand_all()   ) );
  connect( pb_collapse_all, SIGNAL( clicked()      ),
	   this,            SLOT(   collapse_all()   ) ); 
    
  //rightLayout: genTree
  QLabel*      lb_gentree  = us_banner(      tr( "General Report Profile Settings:" ), 1 );
  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  genTree = new QTreeWidget();
  QStringList theads;
  theads << "Selected" << "Protocol Settings";
  genTree->setHeaderLabels( theads );
  genTree->setFont( QFont( US_Widgets::fixedFont().family(),
			      US_GuiSettings::fontSize() + 1 ) );
  genTree->installEventFilter   ( this );
  genTreeLayout->addWidget( lb_gentree );
  genTreeLayout->addWidget( genTree );
  
  genTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  //rightLayout: perChannel tree
  QLabel*      lb_chantree  = us_banner(      tr( "Per-Triple Report Profile Settings:" ), 1 );
  perChanTree = new QTreeWidget();
  QStringList chan_theads;
  chan_theads << "Selected" << "Protocol Settings";
  perChanTree->setHeaderLabels( theads );
  perChanTree->setFont( QFont( US_Widgets::fixedFont().family(),
			       US_GuiSettings::fontSize() + 1 ) );
  perChanTreeLayout->addWidget( lb_chantree );
  perChanTreeLayout->addWidget( perChanTree );
  perChanTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  
  // put layouts together for overall layout
  leftLayout->addLayout( buttonsLayout );
  leftLayout->addStretch();
  rghtLayout->addLayout( genTreeLayout );
  rghtLayout->addLayout( perChanTreeLayout );

  mainLayout->addLayout( leftLayout );
  mainLayout->addLayout( rghtLayout );
  mainLayout->setStretchFactor( leftLayout, 6 );
  mainLayout->setStretchFactor( rghtLayout, 8 );
  
  resize( 1350, 800 );
}

//load GMP run
void US_ReporterGMP::load_gmp_run ( void )
{
  list_all_autoflow_records( autoflowdata  );

  QString pdtitle( tr( "Select GMP Run" ) );
  QStringList hdrs;
  int         prx;
  hdrs << "ID"
       << "Run Name"
       << "Optima Name"
       << "Created"
       << "Run Status"
       << "Stage"
       << "GMP";
  
  QString autoflow_btn = "AUTOFLOW_GMP_REPORT";

  pdiag_autoflow = new US_SelectItem( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );

  QString autoflow_id_selected("");
  if ( pdiag_autoflow->exec() == QDialog::Accepted )
    {
      autoflow_id_selected  = autoflowdata[ prx ][ 0 ];

      //reset Gui && internal structures
      reset_report_panel();
    }
  else
    return;

  
  //show progress dialog
  progress_msg = new QProgressDialog ("Accessing run's protocol...", QString(), 0, 7, this);
  progress_msg->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setModal( true );
  progress_msg->setWindowTitle(tr("Assessing Run's Protocol"));
  QFont font_d  = progress_msg->property("font").value<QFont>();
  QFontMetrics fm(font_d);
  int pixelsWide = fm.width( progress_msg->windowTitle() );
  qDebug() << "Progress_msg: pixelsWide -- " << pixelsWide;
  progress_msg ->setMinimumWidth( pixelsWide*2 );
  progress_msg->adjustSize();
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();
  
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  
  int autoflowID = autoflow_id_selected.toInt();
  protocol_details = read_autoflow_record( autoflowID );
  
  protocol_details[ "autoflowID" ] = QString::number(autoflowID);

  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();
  runID              = protocol_details[ "runID" ];
  FileName           = protocol_details[ "filename" ];

  progress_msg->setValue( 1 );
  qApp->processEvents();

  qDebug() << "1.ExpAborted: "      << protocol_details[ "expAborted" ];
  qDebug() << "1.CorrectRadii: "    << protocol_details[ "correctRadii" ];

  qDebug() << "Exp. Label: "    << protocol_details[ "label" ];
  qDebug() << "GMP Run ? "      << protocol_details[ "gmpRun" ];

  qDebug() << "AnalysisIDs: "   << protocol_details[ "analysisIDs" ];
  qDebug() << "aprofileguid: "  << AProfileGUID ;
  

  //Now, read protocol's 'reportMask' && reportItems masks && populate trees
  read_protocol_and_reportMasks( );

  build_genTree();  
  progress_msg->setValue( 6 );
  qApp->processEvents();

  build_perChanTree();
  progress_msg->setValue( 7 );
  qApp->processEvents();

  progress_msg->setValue( progress_msg->maximum() );
  qApp->processEvents();
  progress_msg->close();

  //Enable some buttons
  le_loaded_run   ->setText( protocol_details[ "filename" ] );
  pb_gen_report   ->setEnabled( true );
  pb_view_report  ->setEnabled( false );
  pb_select_all   ->setEnabled( true );
  pb_unselect_all ->setEnabled( true );
  pb_expand_all   ->setEnabled( true );
  pb_collapse_all ->setEnabled( true );
}

// Query autoflow (history) table for records
int US_ReporterGMP::list_all_autoflow_records( QList< QStringList >& autoflowdata )
{
  int nrecs        = 0;   
  autoflowdata.clear();
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return nrecs;
    }
  
  QStringList qry;
  qry << "get_autoflow_desc";
  db->query( qry );

  while ( db->next() )
    {
      QStringList autoflowentry;
      QString id                 = db->value( 0 ).toString();
      QString runname            = db->value( 5 ).toString();
      QString status             = db->value( 8 ).toString();
      QString optimaname         = db->value( 10 ).toString();
      
      QDateTime time_started     = db->value( 11 ).toDateTime().toUTC();

      QDateTime time_created     = db->value( 13 ).toDateTime().toUTC();
      QString gmpRun             = db->value( 14 ).toString();
      QString full_runname       = db->value( 15 ).toString();
      
      QDateTime local(QDateTime::currentDateTime());

      autoflowentry << id << full_runname << optimaname  << time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );

      if ( time_started.toString().isEmpty() )
	autoflowentry << QString( tr( "NOT STARTED" ) );
      else
	{
	  if ( status == "LIVE_UPDATE" )
	    autoflowentry << QString( tr( "RUNNING" ) );
	  if ( status == "EDITING" || status == "EDIT_DATA" || status == "ANALYSIS" || status == "REPORT" )
	    autoflowentry << QString( tr( "COMPLETED" ) );
	    //autoflowentry << time_started.toString();
	}

      if ( status == "EDITING" )
	status = "LIMS_IMPORT";
      
      autoflowentry << status << gmpRun;

      if ( !full_runname.isEmpty() )
	autoflowdata  << autoflowentry;

      nrecs++;
    }

  return nrecs;
}


// Query autoflow for # records
QMap< QString, QString>  US_ReporterGMP::read_autoflow_record( int autoflowID  )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   QMap <QString, QString> protocol_details;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return protocol_details;
     }

   QStringList qry;
   qry << "read_autoflow_record"
       << QString::number( autoflowID );
   
   db->query( qry );

   if ( db->lastErrno() == US_DB2::OK )      // Autoflow record exists
     {
       while ( db->next() )
	 {
	   protocol_details[ "protocolName" ]   = db->value( 0 ).toString();
	   protocol_details[ "CellChNumber" ]   = db->value( 1 ).toString();
	   protocol_details[ "TripleNumber" ]   = db->value( 2 ).toString();
	   protocol_details[ "duration" ]       = db->value( 3 ).toString();
	   protocol_details[ "experimentName" ] = db->value( 4 ).toString();
	   protocol_details[ "experimentId" ]   = db->value( 5 ).toString();
	   protocol_details[ "runID" ]          = db->value( 6 ).toString();
	   protocol_details[ "status" ]         = db->value( 7 ).toString();
           protocol_details[ "dataPath" ]       = db->value( 8 ).toString();   
	   protocol_details[ "OptimaName" ]     = db->value( 9 ).toString();
	   protocol_details[ "runStarted" ]     = db->value( 10 ).toString();
	   protocol_details[ "invID_passed" ]   = db->value( 11 ).toString();

	   protocol_details[ "correctRadii" ]   = db->value( 13 ).toString();
	   protocol_details[ "expAborted" ]     = db->value( 14 ).toString();
	   protocol_details[ "label" ]          = db->value( 15 ).toString();
	   protocol_details[ "gmpRun" ]         = db->value( 16 ).toString();

	   protocol_details[ "filename" ]       = db->value( 17 ).toString();
	   protocol_details[ "aprofileguid" ]   = db->value( 18 ).toString();

	   protocol_details[ "analysisIDs" ]   = db->value( 19 ).toString();
	   	   
	 }
     }

   return protocol_details;
}

//read protocol's rpeortMask
void US_ReporterGMP::read_protocol_and_reportMasks( void )
{
  //read protocol into US_RunProtocol structure:
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }


  //read protocol
  progress_msg->setValue( 2 );
  qApp->processEvents();
  QString xmlstr( "" );
  US_ProtocolUtil::read_record_auto( ProtocolName_auto, invID,  &xmlstr, NULL, &db );
  QXmlStreamReader xmli( xmlstr );
  currProto. fromXml( xmli );
  progress_msg->setValue( 3 );
  qApp->processEvents();
  
  //Debug: protocol
  qDebug() << "Protocols' details: -- "
	   << currProto.investigator
	   << currProto.runname
	   << currProto.protoname
	   << currProto.protoID
	   << currProto.project
	   << currProto.temperature
	   << currProto.temeq_delay
	   << currProto.exp_label;

  //read AProfile into US_AnaProfile structure
  sdiag = new US_AnalysisProfileGui;
  sdiag->inherit_protocol( &currProto );
  progress_msg->setValue( 4 );
  qApp->processEvents();
  
  currAProf              = sdiag->currProf;
  currAProf.protoGUID    = currProto.protoGUID;
  currAProf.protoID      = currProto.protoID;
  currAProf.protoname    = currProto.protoname;
  //2DSA parms
  cAP2                   = currAProf.ap2DSA;
  //PCSA parms
  cAPp                   = currAProf.apPCSA;
  //Channel descriptions
  chndescs               = currAProf.chndescs;
  //Channel alt_descriptions
  chndescs_alt           = currAProf.chndescs_alt;
  //Channel reports
  ch_reports             = currAProf.ch_reports;
  ch_reports_internal    = currAProf.ch_reports;
  //Channel wavelengths
  ch_wvls                = currAProf.ch_wvls;

  //Debug: AProfile
  QString channel_desc_alt = chndescs_alt[ 0 ];
  QString channel_desc     = chndescs[ 0 ];
  QString wvl              = QString::number( ch_wvls[ channel_desc ][ 0 ] );
  US_ReportGMP reportGMP   = ch_reports[ channel_desc_alt ][ wvl ];
    
  qDebug() << "AProfile's && ReportGMP's details: -- "
	   << currAProf.aprofname
	   << currAProf.protoname
	   << currAProf.chndescs
	   << currAProf.chndescs_alt
	   << currAProf.lc_ratios
	   << cAP2.parms[ 0 ].channel
	   << cAPp.parms[ 0 ].channel
	   << reportGMP.rmsd_limit
	   << reportGMP.wavelength
	   << reportGMP.reportItems[ 0 ].type;

  qDebug() << "Number of wvls in channel: " << chndescs_alt[ 0 ] << ": " <<  ch_wvls[ channel_desc_alt ].size();
  qDebug() << "Wvls in channel: " << chndescs_alt[ 0 ] << ": " << ch_wvls[ channel_desc_alt ];

  
  //report Mask
  QString gen_reportMask = currAProf.report_mask;
  parse_gen_mask_json( gen_reportMask );

  progress_msg->setValue( 5 );
  qApp->processEvents();

  qDebug() << "General ReportMask: " << gen_reportMask;
}

//parse JSON for general rpeort mask
void US_ReporterGMP::parse_gen_mask_json ( const QString reportMask  )
{
  QJsonDocument jsonDoc = QJsonDocument::fromJson( reportMask.toUtf8() );
  json = jsonDoc.object();

  topLevelItems = json.keys();
  
  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();
      
      if ( key.contains("Solutions") || key.contains("Analysis") )
	{
	   QJsonArray json_array = value.toArray();
	   for (int i=0; i < json_array.size(); ++i )
	     {
	       foreach(const QString& array_key, json_array[i].toObject().keys())
		 {
		   if (  key.contains("Solutions") )
		     {
		       solutionItems      << array_key;
		       solutionItems_vals << json_array[i].toObject().value(array_key).toString(); 
		     }
		   if (  key.contains("Analysis") )
		     {
		       QJsonObject newObj = json_array[i].toObject().value(array_key).toObject();
		       analysisItems << array_key;

		       foreach ( const QString& n_key, newObj.keys() )
			 {
			   if ( array_key.contains("General") )
			     {
			       analysisGenItems << n_key;
			       analysisGenItems_vals << newObj.value( n_key ).toString();
			     }
			   if ( array_key.contains("2DSA") )
			     {
			       analysis2DSAItems << n_key;
			       analysis2DSAItems_vals << newObj.value( n_key ).toString();
			     }
			   if ( array_key.contains("PCSA") ) 
			     {
			       analysisPCSAItems << n_key;
			       analysisPCSAItems_vals << newObj.value( n_key ).toString();
			     }
			 }
		     }
		 }
	     }
	}
    }

  qDebug() << "solutionItems: " << solutionItems;
  qDebug() << "solutionItems_vals: " << solutionItems_vals;

  qDebug() << "analysisItems: " << analysisItems;
  
  qDebug() << "analysisGenItems: " << analysisGenItems;
  qDebug() << "analysisGenItems_vals: " << analysisGenItems_vals;

  qDebug() << "analysis2DSAItems: " << analysis2DSAItems;
  qDebug() << "analysis2DSAItems_vals: " << analysis2DSAItems_vals;

  qDebug() << "analysisPCSAItems: " << analysisPCSAItems;
  qDebug() << "analysisPCSAItems_vals: " << analysisPCSAItems_vals;

}

//build general report mask tree
void US_ReporterGMP::build_genTree ( void )
{
  QString indent( "  " );
  QStringList topItemNameList, solutionItemNameList, analysisItemNameList,
              analysisGenItemNameList, analysis2DSAItemNameList, analysisPCSAItemNameList;
  int wiubase = (int)QTreeWidgetItem::UserType;

  for ( int i=0; i<topLevelItems.size(); ++i )
    {
      QString topItemName = topLevelItems[i];
      topItemNameList.clear();
      topItemNameList << "" << indent + topItemName;
      topItem [ topItemName ] = new QTreeWidgetItem( genTree, topItemNameList, wiubase );

      //Solutions: add 1-level children
      if( topItemName.contains("Solutions") )
	{
	  int checked_childs = 0;
	  for ( int is=0; is<solutionItems.size(); ++is )
	    {
	      QString solutionItemName = solutionItems[ is ];
	      solutionItemNameList.clear();
	      solutionItemNameList << "" << indent.repeated( 2 ) + solutionItemName;
	      solutionItem [ solutionItemName ] = new QTreeWidgetItem( topItem [ topItemName ], solutionItemNameList, wiubase);

	      if ( solutionItems_vals[ is ].toInt() )
		{
		  solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Checked );
		  ++checked_childs;
		}
	      else
		solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Unchecked );
	    }
	  if ( checked_childs )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	  
	}
      
      //Analysis Profile: add 2-levelchildren
      else if ( topItemName.contains("Analysis Profile") )
	{
	  int checked_childs = 0;
	  for ( int ia=0; ia < analysisItems.size(); ++ia )
	    {
	      QString analysisItemName = analysisItems[ ia ];
	      analysisItemNameList.clear();
	      analysisItemNameList << "" << indent.repeated( 2 ) + analysisItemName;
	      analysisItem [ analysisItemName ] = new QTreeWidgetItem( topItem [ topItemName ], analysisItemNameList, wiubase);
	      
	      //General analysis
	      if( analysisItemName.contains("General") )
		{
		  int checked_gen = 0;
		  for ( int iag=0; iag < analysisGenItems.size(); ++iag )
		    {
		      QString analysisGenItemName = analysisGenItems[ iag ];
		      analysisGenItemNameList.clear();
		      analysisGenItemNameList << "" << indent.repeated( 3 ) + analysisGenItemName;
		      analysisGenItem [ analysisGenItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisGenItemNameList, wiubase);

		      if ( analysisGenItems_vals[ iag ].toInt() )
			{
			  analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_gen;
			}
		      else
			analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_gen )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}
	      //2DSA analysis
	      if( analysisItemName.contains("2DSA") )
		{
		  int checked_2dsa = 0;
		  for ( int ia2=0; ia2 < analysis2DSAItems.size(); ++ia2 )
		    {
		      QString analysis2DSAItemName = analysis2DSAItems[ ia2 ];
		      analysis2DSAItemNameList.clear();
		      analysis2DSAItemNameList << "" << indent.repeated( 3 ) + analysis2DSAItemName;
		      analysis2DSAItem [ analysis2DSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysis2DSAItemNameList, wiubase);

		      if ( analysis2DSAItems_vals[ ia2 ].toInt() )
			{
			  analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_2dsa;
			}
		      else
			analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_2dsa )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}

	      //PCSA analysis
	      if( analysisItemName.contains("PCSA") )
		{
		  int checked_pcsa = 0;
		  for ( int iap=0; iap < analysisPCSAItems.size(); ++iap )
		    {
		      QString analysisPCSAItemName = analysisPCSAItems[ iap ];
		      analysisPCSAItemNameList.clear();
		      analysisPCSAItemNameList << "" << indent.repeated( 3 ) + analysisPCSAItemName;
		      analysisPCSAItem [ analysisPCSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisPCSAItemNameList, wiubase);

		      if ( analysisPCSAItems_vals[ iap ].toInt() )
			{
			  analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_pcsa;
			}
		      else
			analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_pcsa )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}
	    }
	  if ( checked_childs )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	}

      //set checked/unchecked for top-level item
      else
	{
	  if ( json.value( topItemName ).toString().toInt() )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	}
    }

  
  genTree->expandAll();    
  genTree->resizeColumnToContents( 0 );
  genTree->resizeColumnToContents( 1 );

  connect( genTree, SIGNAL( itemChanged    ( QTreeWidgetItem*, int ) ),
  	   this,    SLOT  ( changedItem_gen( QTreeWidgetItem*, int ) ) );

}

//What to check/uncheck upon change in items status
void US_ReporterGMP::changedItem_gen( QTreeWidgetItem* item, int col )
{
  if ( col == 0  ) //we deal with col 0 only...
    {
      //qDebug() << "Changed item name0" << item->text( 1 );

      //if has (nested) children items
      int children_lev1 = item->childCount();
      if ( children_lev1 )
	{
	  genTree -> disconnect();

	  for( int i = 0; i < children_lev1; ++i )
	    {
	      item->child(i)->setCheckState( 0, (Qt::CheckState) item->checkState(0) );

	      int children_lev2 = item->child(i)->childCount();
	      if ( children_lev2 )
		{
		  for( int ii = 0; ii < children_lev2; ++ii )
		    {
		      item->child(i)->child(ii)->setCheckState( 0, (Qt::CheckState) item->child(i)->checkState(0) );
		    }
		}
	    }
	  
	  connect( genTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	           this,       SLOT(   changedItem_gen( QTreeWidgetItem*, int ) ) );
	}
      
           
      //qDebug() << "Changed item name1 " << item->text( 1 );
      	
      //if has parent item
      QTreeWidgetItem* parent_item = item->parent();

      //qDebug() << "Changed item name2: " << item->text( 1 );
      
      if ( parent_item )
	{
	  //qDebug() << "Changed item name3: " << item->text( 1 );
	  genTree -> disconnect();
	  
	  //qDebug() << " Current item, " << item->text( 1 ) << ", has a parent: " << parent_item->text( 1 );
	    
	  int checked_children = 0;
	  int parent_item_children = parent_item ->childCount();
	  for( int i = 0; i < parent_item_children; ++i )
	    {
	      if ( int( parent_item->child( i )->checkState(0) ) )
		++checked_children;
	    }
	  if ( checked_children )
	    parent_item->setCheckState( 0, Qt::Checked );
	  else
	    parent_item->setCheckState( 0, Qt::Unchecked );

	  connect( genTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	           this,    SLOT(   changedItem_gen( QTreeWidgetItem*, int ) ) );
	}
    }
}

//build perChanTree
void US_ReporterGMP::build_perChanTree ( void )
{
  QString indent( "  " );
  QStringList chanItemNameList, tripleItemNameList, tripleMaskItemNameList;
  QStringList tripleReportMasksList;
  QList< bool > tripleReportMasksList_vals;
  int wiubase = (int)QTreeWidgetItem::UserType;
  
  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = chndescs_alt[ i ];
      QString channel_desc     = chndescs[ i ];


      bool triple_report = false;
      
      if ( currAProf.analysis_run[ i ] )
	{
	  //now check if report will be run:
	  QString run_report;
	  if ( currAProf.report_run[ i ] )
	    triple_report = true;
	}

      if ( triple_report )
	{
	  // Channel Name: topItem in a perChanTree
	  QString chanItemName = "Channel: " + channel_desc_alt.section( ":", 0, 1 );
	  chanItemNameList.clear();
	  chanItemNameList << "" << indent + chanItemName;
	  chanItem [ chanItemName ] = new QTreeWidgetItem( perChanTree, chanItemNameList, wiubase );
	  
	  //QList < double > chann_wvls                  = ch_wvls[ channel_desc ];
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc_alt ];
	  QMap < QString, US_ReportGMP > chann_reports = ch_reports[ channel_desc_alt ];

	  qDebug() << "channel_desc_alt , channel_desc: "   << channel_desc_alt << " , " <<  channel_desc;
	  qDebug() << "chann_wvls [ channel_desc ] -- "     << chann_wvls;
	  qDebug() << "chann_wvls [ channel_desc_alt ] -- " << ch_wvls[ channel_desc_alt ];
	  
	  int chann_wvl_number = chann_wvls.size();

	  int checked_triples = 0;
	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl            = QString::number( chann_wvls[ jj ] );
	      QString triple_name    = channel_desc.split(":")[ 0 ] + "/" + wvl;

	      //Push to Array_of_triples;
	      QString tripleName = channel_desc_alt.section( ":", 0, 0 )[0] + "." + channel_desc_alt.section( ":", 0, 0 )[1] + "." + wvl;
	      qDebug() << "TripleName -- " << tripleName; 
	      Array_of_triples.push_back( tripleName );

	      //Triple item: child-level 1 in a perChanTree
	      QString tripleItemName = "Triple:  " + wvl + " nm";
	      tripleItemNameList.clear();
	      tripleItemNameList << "" << indent.repeated( 2 ) + tripleItemName;
	      tripleItem [ tripleItemName ] = new QTreeWidgetItem( chanItem [ chanItemName ], tripleItemNameList, wiubase);

	      US_ReportGMP reportGMP = chann_reports[ wvl ];
	      
	      qDebug() << reportGMP. tot_conc_mask
		       << reportGMP. rmsd_limit_mask
		       << reportGMP. av_intensity_mask
		       << reportGMP. experiment_duration_mask
		       << reportGMP. integration_results_mask;

	      tripleReportMasksList.clear();
	      tripleReportMasksList << "Total Concentration"
				    << "RMSD Limit"
				    << "Minimum Intensity"
				    << "Experiment Duration"
				    << "Integraiton Results";
	      
	      tripleReportMasksList_vals.clear();
	      tripleReportMasksList_vals << reportGMP. tot_conc_mask
	      				 << reportGMP. rmsd_limit_mask
	      				 << reportGMP. av_intensity_mask
	      				 << reportGMP. experiment_duration_mask
	      				 << reportGMP. integration_results_mask;

	      int checked_masks = 0;
	      for ( int kk = 0; kk < tripleReportMasksList.size(); ++kk )
		{
		  //Triple's mask params: child-level 2 in a perChanTree
		  QString tripleMaskItemName = tripleReportMasksList[ kk ];
		  tripleMaskItemNameList.clear();
		  tripleMaskItemNameList << "" << indent.repeated( 3 ) + tripleMaskItemName;
		  tripleMaskItem [ tripleItemName ] = new QTreeWidgetItem(  tripleItem [ tripleItemName ], tripleMaskItemNameList, wiubase);

		  if ( tripleReportMasksList_vals[ kk ] )
		    {
		      tripleMaskItem [ tripleItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_masks;
		    }
		  else
		    tripleMaskItem [ tripleItemName ] ->setCheckState( 0, Qt::Unchecked );
		}

	      if ( checked_masks )
		{
		  tripleItem [ tripleItemName ] ->setCheckState( 0, Qt::Checked );
		  ++checked_triples;
		}
	      else
		tripleItem [ tripleItemName ] ->setCheckState( 0, Qt::Unchecked );
	    }
	  
	  //set checked/unchecked for channel (parent)
	  if ( checked_triples )
	    chanItem [ chanItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    chanItem [ chanItemName ] ->setCheckState( 0, Qt::Unchecked );
	}
    }

  perChanTree->expandAll();    
  perChanTree->resizeColumnToContents( 0 );
  perChanTree->resizeColumnToContents( 1 );

  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	   this,        SLOT(   changedItem_triple( QTreeWidgetItem*, int ) ) );
}

//What to check/uncheck upon change in items status
void US_ReporterGMP::changedItem_triple( QTreeWidgetItem* item, int col )
{
  if ( col == 0  ) //we deal with col 0 only...
    {
      //qDebug() << "Changed item name0" << item->text( 1 );

      //if has (nested) children items
      int children_lev1 = item->childCount();
      if ( children_lev1 )
	{
	  perChanTree -> disconnect();

	  for( int i = 0; i < children_lev1; ++i )
	    {
	      item->child(i)->setCheckState( 0, (Qt::CheckState) item->checkState(0) );

	      int children_lev2 = item->child(i)->childCount();
	      if ( children_lev2 )
		{
		  for( int ii = 0; ii < children_lev2; ++ii )
		    {
		      item->child(i)->child(ii)->setCheckState( 0, (Qt::CheckState) item->child(i)->checkState(0) );
		    }
		}
	    }
	  
	  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	           this,        SLOT(   changedItem_triple( QTreeWidgetItem*, int ) ) );
	}
      
           
      //qDebug() << "Changed item name1 " << item->text( 1 );
      	
      //if has parent item
      QTreeWidgetItem* parent_item = item->parent();

      //qDebug() << "Changed item name2: " << item->text( 1 );
      
      if ( parent_item )
	{
	  //qDebug() << "Changed item name3: " << item->text( 1 );
	  perChanTree -> disconnect();
	  
	  //qDebug() << " Current item, " << item->text( 1 ) << ", has a parent: " << parent_item->text( 1 );
	    
	  int checked_children = 0;
	  int parent_item_children = parent_item ->childCount();
	  for( int i = 0; i < parent_item_children; ++i )
	    {
	      if ( int( parent_item->child( i )->checkState(0) ) )
		++checked_children;
	    }
	  if ( checked_children )
	    parent_item->setCheckState( 0, Qt::Checked );
	  else
	    parent_item->setCheckState( 0, Qt::Unchecked );

	  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	           this,        SLOT(   changedItem_triple( QTreeWidgetItem*, int ) ) );
	}
    }
}


//select all items in trees
void US_ReporterGMP::select_all ( void )
{
  QTreeWidgetItem* getTree_rootItem     = genTree     -> invisibleRootItem();
  for( int i = 0; i < getTree_rootItem->childCount(); ++i )
    {
      getTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
      getTree_rootItem->child(i)->setCheckState( 0, Qt::Checked );
    }
  
  QTreeWidgetItem* perChanTree_rootItem = perChanTree -> invisibleRootItem();
  for( int i = 0; i < perChanTree_rootItem->childCount(); ++i )
    {
      perChanTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
      perChanTree_rootItem->child(i)->setCheckState( 0, Qt::Checked );
    }
}


//unselect all items in trees
void US_ReporterGMP::unselect_all ( void )
{
  QTreeWidgetItem* getTree_rootItem     = genTree     -> invisibleRootItem();
  for( int i = 0; i < getTree_rootItem->childCount(); ++i )
    {
      getTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
    }
  
  QTreeWidgetItem* perChanTree_rootItem = perChanTree -> invisibleRootItem();
  for( int i = 0; i < perChanTree_rootItem->childCount(); ++i )
    {
      perChanTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
    }
}

//expand all items in trees
void US_ReporterGMP::expand_all ( void )
{
  genTree     ->expandAll();
  perChanTree ->expandAll();
}

//collapse all items in trees
void US_ReporterGMP::collapse_all ( void )
{
  genTree       ->collapseAll();
  perChanTree   ->collapseAll();
}

//view report
void US_ReporterGMP::view_report ( void )
{
  qDebug() << "Opening PDF at -- " << filePath;
  
  //Open with OS's applicaiton settings ?
  QDesktopServices::openUrl(QUrl( filePath ));
}

//reset
void US_ReporterGMP::reset_report_panel ( void )
{
  le_loaded_run ->setText( "" );

  //cleaning genTree && it's objects
  // for (int i = 0; i < genTree->topLevelItemCount(); ++i)
  //   {
  //     qDeleteAll(genTree->topLevelItem(i)->takeChildren());
  //   }
  genTree     ->clear();
  genTree     -> disconnect();
  qApp->processEvents();
  
  topItem.clear();
  solutionItem.clear();
  analysisItem.clear();
  analysisGenItem.clear();
  analysis2DSAItem.clear();
  analysisPCSAItem.clear();

  topLevelItems.clear();
  solutionItems.clear();
  solutionItems_vals.clear();
  analysisItems.clear();
  analysisGenItems.clear();
  analysisGenItems_vals.clear();
  analysis2DSAItems.clear();
  analysis2DSAItems_vals.clear();
  analysisPCSAItems.clear();
  analysisPCSAItems_vals.clear();

  //cleaning perTriple tree & it's objects
  perChanTree ->clear();
  perChanTree ->disconnect();
  qApp->processEvents();

  chanItem   .clear();
  tripleItem .clear();
  tripleMaskItem . clear();

  //clean triple_array
  Array_of_triples.clear();
  
  //reset US_Protocol && US_AnaProfile
  currProto = US_RunProtocol();  
  currAProf = US_AnaProfile();   

  //reset html assembled strings
  html_assembled.clear();
  html_general.clear();
  html_lab_rotor.clear();
  html_operator.clear();
  html_speed.clear();
  html_cells.clear();
  html_solutions.clear();
  html_optical.clear();
  html_ranges.clear();
  html_scan_count.clear();
  html_analysis_profile.clear();
  html_analysis_profile_2dsa.clear();
  html_analysis_profile_pcsa .clear();

  qApp->processEvents();
}


//Generate report
void US_ReporterGMP::generate_report( void )
{
  progress_msg->setWindowTitle(tr("Generating Report"));
  progress_msg->setLabelText( "Generating report: Part 1..." );
  int msg_range = currProto.rpSolut.nschan + 4;
  progress_msg->setRange( 0, msg_range );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();

  //reset html assembled strings
  html_assembled.clear();
  html_general.clear();
  html_lab_rotor.clear();
  html_operator.clear();
  html_speed.clear();
  html_cells.clear();
  html_solutions.clear();
  html_optical.clear();
  html_ranges.clear();
  html_scan_count.clear();
  html_analysis_profile.clear();
  html_analysis_profile_2dsa.clear();
  html_analysis_profile_pcsa .clear();

  
  //Part 1
  gui_to_parms();
  progress_msg->setValue( 1 );
  qApp->processEvents();
  
  get_current_date();
  progress_msg->setValue( 2 );
  qApp->processEvents();
  
  format_needed_params();
  progress_msg->setValue( 3 );
  qApp->processEvents();

  assemble_pdf();
  progress_msg->setValue( 4 );
  qApp->processEvents();
  
  progress_msg->setValue( progress_msg->maximum() );
  progress_msg->close();
  qApp->processEvents();

  //Part 2
  for ( int i=0; i<Array_of_triples.size(); ++i )
    {
      currentTripleName = Array_of_triples[i];
      simulate_triple ( currentTripleName );
    }

  write_pdf_report( );
  qApp->processEvents();
  
  pb_view_report->setEnabled( true );

  //Inform user of the PDF location
  QMessageBox::information( this, tr( "Report PDF Ready" ),
			    tr( "Report PDF was saved at \n%1\n\n"
				"You can view it by pressing \'View Report\' button on the left" ).arg( filePath ) );
}

//simulate triple 
void US_ReporterGMP::simulate_triple( const QString triplesname )
{
  // Show msg while data downloaded and simulated
  progress_msg = new QProgressDialog (QString("Downloading data and models for triple %1...").arg( triplesname ), QString(), 0, 5, this);
  progress_msg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setWindowModality(Qt::WindowModal);
  progress_msg->setWindowTitle(tr("Simulating Models"));
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();
  
  speed_steps  .clear();
  edata = NULL;
  rdata = NULL;
  //sdata = NULL;
  eID_global = 0;
  sdata          = &wsdata;
  
  dbg_level  = US_Settings::us_debug();

  adv_vals[ "simpoints" ] = "500";
  adv_vals[ "bndvolume" ] = "0.015";
  adv_vals[ "parameter" ] = "0";
  adv_vals[ "modelnbr"  ] = "0";
  adv_vals[ "meshtype"  ] = "ASTFEM";
  adv_vals[ "gridtype"  ] = "Moving";
  adv_vals[ "modelsim"  ] = "mean";

  resids.clear();
  dataLoaded = false;
  buffLoaded = false;
  haveSim    = false;
  resplotd   = 0;       //<--TEMP
  ti_noise.count = 0;
  ri_noise.count = 0;


  QString stage_n   = QString("2DSA-IT");
  QString triple_n  = triplesname;
  //stage_n : '2DSA-IT'
  //triple_n: '2.A.255'

  qDebug() << "In SHOW OVERLAY: triple_stage / triple_name: " << stage_n << " / " << triple_n;

  tripleInfo = ": " + triple_n + " (" + stage_n + ")";

  //Parse filename
  FileName_parsed = get_filename( triple_n );
  qDebug() << "In show_overlay(): FileName_parsed: " << FileName_parsed;
  
  //LoadData
  QMap< QString, QString > triple_info_map;
  triple_info_map[ "triple_name" ]     = triple_n;
  triple_info_map[ "stage_name" ]      = stage_n;
  triple_info_map[ "invID" ]           = QString::number(invID);
  triple_info_map[ "filename" ]        = FileName_parsed;

  dataLoaded = false;
  buffLoaded = false;
  haveSim    = false;
  
  loadData( triple_info_map );
  progress_msg->setValue( 1 );
  
  triple_info_map[ "eID" ]        = QString::number( eID_global );
  // Assign edata && rdata
  edata     = &editedData[ 0 ];
  rdata     = &rawData[ 0 ];

  
  // Get speed steps from DB experiment (and maybe timestate)
  QString tmst_fpath = US_Settings::resultDir() + "/" + FileName_parsed + "/"
    + FileName_parsed + ".time_state.tmst";

  US_Passwd   pw;
  US_DB2*     dbP    = new US_DB2( pw.getPasswd() );
  QStringList query;
  QString     expID;
  int         idExp  = 0;
  query << "get_experiment_info_by_runID"
	<< FileName_parsed
	<< QString::number(invID);
  dbP->query( query );
  
  if ( dbP->lastErrno() == US_DB2::OK )
    {
      dbP->next();
      idExp              = dbP->value( 1 ).toInt();
      US_SimulationParameters::speedstepsFromDB( dbP, idExp, speed_steps );
    }
  
  // Check out whether we need to read TimeState from the DB
  bool newfile       = US_TimeState::dbSyncToLF( dbP, tmst_fpath, idExp );

  //Get speed info
  QFileInfo check_file( tmst_fpath );
  if ( check_file.exists()  &&  check_file.isFile() )
    {  // Get speed_steps from an existing timestate file
      simparams.simSpeedsFromTimeState( tmst_fpath );
      simparams.speedstepsFromSSprof();

      //*DEBUG*
      int essknt=speed_steps.count();
      int tssknt=simparams.speed_step.count();
      qDebug() << "LD: (e)ss knt" << essknt << "(t)ss knt" << tssknt;
      for ( int jj = 0; jj < qMin( essknt, tssknt ); jj++ )
	{
	  qDebug() << "LD:  jj" << jj << "(e) tf tl wf wl scns"
		   << speed_steps[jj].time_first
		   << speed_steps[jj].time_last
		   << speed_steps[jj].w2t_first
		   << speed_steps[jj].w2t_last
		   << speed_steps[jj].scans;
	  qDebug() << "LD:    (t) tf tl wf wl scns"
		   << simparams.speed_step[jj].time_first
		   << simparams.speed_step[jj].time_last
		   << simparams.speed_step[jj].w2t_first
		   << simparams.speed_step[jj].w2t_last
		   << simparams.speed_step[jj].scans;
	}
      //*DEBUG*

      int kstep      = speed_steps.count();
      int kscan      = speed_steps[ 0 ].scans;
      for ( int jj = 0; jj < simparams.speed_step.count(); jj++ )
	{
	  if ( jj < kstep )
	    {
	      kscan          = speed_steps[ jj ].scans;
	      speed_steps[ jj ] = simparams.speed_step[ jj ];
	    }
	  else
            speed_steps << simparams.speed_step[ jj ];
	  
	  speed_steps[ jj ].scans = kscan;
	  simparams.speed_step[ jj ].scans = kscan;
	  qDebug() << "LD:    (s) tf tl wf wl scns"
		   << speed_steps[jj].time_first
		   << speed_steps[jj].time_last
		   << speed_steps[jj].w2t_first
		   << speed_steps[jj].w2t_last
		   << speed_steps[jj].scans;
	}
    }
  progress_msg->setValue( 2 );
  qApp->processEvents();

  dataLoaded = true;
  haveSim    = false;
  scanCount  = edata->scanData.size();
 

  //Read Solution/Buffer
  density      = DENS_20W;
  viscosity    = VISC_20W;
  compress     = 0.0;
  manual       = false;
  QString solID;
  QString bufid;
  QString bguid;
  QString bdesc;
  QString bdens = QString::number( density );
  QString bvisc = QString::number( viscosity );
  QString bcomp = QString::number( compress );
  QString bmanu = manual ? "1" : "0";
  QString svbar = QString::number( 0.7200 );
  bool    bufvl = false;
  QString errmsg;
  qDebug() << "Fem:Upd: (0)svbar" << svbar;
  
  bufvl = US_SolutionVals::values( dbP, edata, solID, svbar, bdens,
				   bvisc, bcomp, bmanu, errmsg );
  progress_msg->setValue( 3 );

  //Hardwire compressibility to zero, for now
  bcomp="0.0";
  if ( bufvl )
    {
      buffLoaded  = false;
      buffLoaded  = true;
      density     = bdens.toDouble();
      viscosity   = bvisc.toDouble();
      compress    = bcomp.toDouble();
      manual      = ( !bmanu.isEmpty()  &&  bmanu == "1" );
      if ( solID.isEmpty() )
	{
	  QMessageBox::warning( this, tr( "Solution/Buffer Fetch" ),
				tr( "Empty solution ID value!" ) );
	}
      
      else if ( solID.length() < 36  &&  dbP != NULL )
	{
	  solution_rec.readFromDB( solID.toInt(), dbP );
	}
      
      else
	{
	  solution_rec.readFromDisk( solID );
	}
      
      vbar          = solution_rec.commonVbar20;
      svbar         = QString::number( vbar );
      
    }
  else
    {
      QMessageBox::warning( this, tr( "Solution/Buffer Fetch" ),
			    errmsg );
      solution_rec.commonVbar20 = vbar;
    }
  
  ti_noise.count = 0;
  ri_noise.count = 0;

  // Calculate basic parameters for other functions [ from us_fematch's ::update()-> data_plot() ]
  double avgTemp     = edata->average_temperature();
  solution.density   = density;
  solution.viscosity = viscosity;
  solution.manual    = manual;
  solution.vbar20    = svbar.toDouble();
  svbar_global       = svbar; 
  solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
  
  US_Math2::data_correction( avgTemp, solution );
  

  //Load Model (latest ) && noise(s)
  triple_info_map[ "stage_name" ] = QString("2DSA-IT");
  if ( !loadModel( triple_info_map  ) && !model_exists )
    {
      triple_info_map[ "stage_name" ] = QString("2DSA-FM");
      if ( !loadModel( triple_info_map  ) && !model_exists )
	{
	  triple_info_map[ "stage_name" ] = QString("2DSA");
	  if ( !loadModel( triple_info_map  ) && !model_exists )
	    {
	      qDebug() << "In loadModel(): No models (2DSA-IT, 2DSA-FM, 2DSA) found for triple -- " << triple_info_map[ "triple_name" ];

	      QMessageBox::critical( this, tr( "No Model Found" ),
				     QString( tr( "In loadModel(): No models (2DSA-IT, 2DSA-FM, 2DSA) found for triple %1" ))
				     .arg( triple_info_map[ "triple_name" ] ) );
	      
	      return;
	    }
	}
    }

  progress_msg->setValue( 5 );
  qApp->processEvents();

  //Simulate Model
  simulateModel( triple_info_map );

  
  qDebug() << "Closing sim_msg-- ";
  //msg_sim->accept();
  progress_msg->close();

  /*
  // Show plot
  resplotd = new US_ResidPlotFem( this, true );
  //resplotd->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  resplotd->setWindowModality(Qt::ApplicationModal);
  resplotd->show();
  */
}

// public function to get pointer to edit data
US_DataIO::EditedData*      US_ReporterGMP::rg_editdata() { return edata;     }

// public function to get pointer to list of excluded scans
QList< int >*               US_ReporterGMP::rg_excllist() { return &excludedScans;}

// public function to get pointer to sim data
US_DataIO::RawData*         US_ReporterGMP::rg_simdata()  { return sdata;     }

// public function to get pointer to load model
US_Model*                   US_ReporterGMP::rg_model()    { return &model;    }

// public function to get pointer to TI noise
US_Noise*                   US_ReporterGMP::rg_ti_noise() { return &ti_noise; }

// public function to get pointer to RI noise
US_Noise*                   US_ReporterGMP::rg_ri_noise() { return &ri_noise; }

// public function to get pointer to resid bitmap diag
QPointer< US_ResidsBitmap > US_ReporterGMP::rg_resbmap()  { return rbmapd;    }

QString  US_ReporterGMP::rg_tripleInfo()  { return tripleInfo;    }


//Load rawData/editedData
bool US_ReporterGMP::loadData( QMap < QString, QString > & triple_information )
{
  rawData.clear();
  editedData.clear();
  
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return false;
    }

  int rID=0;
  QString rfilename;
  int eID=0;
  QString efilename;
  
  //get EditedData filename && editedDataID for current triple, then infer rawDataID 
  QStringList query;
  query << "get_editedDataFilenamesIDs" << triple_information["filename"];
  db->query( query );

  qDebug() << "In loadData() Query: " << query;
  qDebug() << "In loadData() Query: triple_information[ \"triple_name\" ]  -- " << triple_information[ "triple_name" ];

  int latest_update_time = 1e100;

  QString triple_name_actual = triple_information[ "triple_name" ];

  if ( triple_name_actual.contains("Interference") )
    triple_name_actual.replace( "Interference", "660" );
  
  while ( db->next() )
    {
      QString  filename            = db->value( 0 ).toString();
      int      editedDataID        = db->value( 1 ).toInt();
      int      rawDataID           = db->value( 2 ).toInt();
      //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
      QDateTime date               = db->value( 3 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();
               
      if ( filename.contains( triple_name_actual ) ) 
	{
	  int time_to_now = date.secsTo(now);
	  if ( time_to_now < latest_update_time )
	    {
	      latest_update_time = time_to_now;
	      //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

	      rID       = rawDataID;
	      eID       = editedDataID;
	      efilename = filename;
	    }
	}
    }

  qDebug() << "In loadData() after Query ";
  
  QString edirpath  = US_Settings::resultDir() + "/" + triple_information[ "filename" ];
  QDir edir( edirpath );
  if (!edir.exists())
    edir.mkpath( edirpath );
  
  QString efilepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + efilename;

  qDebug() << "In loadData() efilename: " << efilename;

  
  // Can check here if such filename exists
  // QFileInfo check_file( efilepath );
  // if ( check_file.exists() && check_file.isFile() )
  //   qDebug() << "EditProfile file: " << efilepath << " exists";
  // else
  db->readBlobFromDB( efilepath, "download_editData", eID );

  qDebug() << "In loadData() after readBlobFromDB ";

  //Now download rawData corresponding to rID:
  QString efilename_copy = efilename;
  QStringList efilename_copy_list = efilename_copy.split(".");

  rfilename = triple_information[ "filename" ] + "." + efilename_copy_list[2] + "."
                                               + efilename_copy_list[3] + "."
                                               + efilename_copy_list[4] + "."
                                               + efilename_copy_list[5] + ".auc";
  
  QString rfilepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + rfilename;
  //do we need to check for existance ?
  db->readBlobFromDB( rfilepath, "download_aucData", rID );

  qApp->processEvents();

  qDebug() << "Loading eData, rawData: efilepath, rfilepath, eID, rID --- " << efilepath << rfilepath << eID << rID;

  //Put downloaded data in memory:
  QString uresdir = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/"; 
  US_DataIO::loadData( uresdir, efilename, editedData, rawData );

  eID_global = eID;

  qDebug() << "END of loadData(), eID_global: " << eID_global;

  return true;
}

//Load rawData/editedData
bool US_ReporterGMP::loadModel( QMap < QString, QString > & triple_information )
{
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return false;
    }

  //first, get ModelIDs corresponding to editedDataID AND triple_stage && select latest one
  QStringList query;
  query << "get_modelDescsIDs" << triple_information[ "eID" ];
  db->query( query );
  
  qDebug() << "In loadModel() Query: " << query;
  
  int latest_update_time = 1e100;
  int mID=0;

  model_exists = false;
  
  while ( db->next() )
    {
      QString  description         = db->value( 0 ).toString();
      int      modelID             = db->value( 1 ).toInt();
      //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
      QDateTime date               = db->value( 2 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();
      
      if ( description.contains( triple_information[ "stage_name" ] ) ) 
	{
	  //if contains, it matches & the model exists (e.g. 2DSA-IT); now find the latest one
	  model_exists = true;
	  
	  if ( triple_information[ "stage_name" ] == "2DSA" )
	    {
	      if ( !description.contains("-FM_") && !description.contains("-IT_") && !description.contains("-MC_") && !description.contains("_mcN") )
		{
		  int time_to_now = date.secsTo(now);
		  if ( time_to_now < latest_update_time )
		    {
		      latest_update_time = time_to_now;
		      //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

		      qDebug() << "Model 2DSA: ID, desc, timetonow -- " << modelID << description << time_to_now;
		  		      
		      mID       = modelID;
		    }
		}
	    }
	  else
	    {
	      int time_to_now = date.secsTo(now);
	      if ( time_to_now < latest_update_time )
		{
		  latest_update_time = time_to_now;
		  //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);
		  
		  qDebug() << "Model NON-2DSA: ID, desc, timetonow -- " << modelID << description << time_to_now;
		  
		  mID       = modelID;
		}
	    }
	}
    }

  if ( ! model_exists )
    {
      // QMessageBox::critical( this, tr( "Model Does Not Exists!" ),
      // 			     QString (tr( "Triple %1 does not have  %2 model !" ))
      // 			     .arg( triple_information[ "triple_name" ] )
      // 			     .arg( triple_information[ "stage_name" ] ) );

      progress_msg->setLabelText( QString("Model %1 is NOT found for triple %2.\n Trying other models...")
				  .arg( triple_information[ "stage_name" ] )
				  .arg( triple_information[ "triple_name" ] ) );
      progress_msg->setValue( 4 );
      qApp->processEvents();
      
      return false;
    }
  
  int  rc      = 0;
  qDebug() << "ModelID to retrieve: -- " << mID;
  rc   = model.load( QString::number( mID ), db );
  qDebug() << "LdM:  model load rc" << rc;
  qApp->processEvents();

  model_loaded = model;   // Save model exactly as loaded
  model_used   = model;   // Make that the working model
  is_dmga_mc   = ( model.monteCarlo  &&
		   model.description.contains( "DMGA" )  &&
		   model.description.contains( "_mcN" ) );
  qDebug() << "post-Load mC" << model.monteCarlo << "is_dmga_mc" << is_dmga_mc
	   << "description" << model.description;
  
  if ( model.components.size() == 0 )
    {
      QMessageBox::critical( this, tr( "Empty Model" ),
			     tr( "Loaded model has ZERO components!" ) );
      return false;
    }
  
  ti_noise.count = 0;
  ri_noise.count = 0;
  ti_noise.values.clear();
  ri_noise.values.clear();

  //Load noise files
  triple_information[ "mID" ] = QString::number( mID );

  progress_msg->setValue( 4 );
  qApp->processEvents();

  loadNoises( triple_information );
  
  return true;
}

//Load Noises
bool US_ReporterGMP::loadNoises( QMap < QString, QString > & triple_information )
{
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return false;
    }

  // get noiseIDs, types & lastUpd by modelID
  QStringList query;
  query << "get_noiseTypesIDs" << triple_information[ "mID" ];
  db->query( query );
  
  qDebug() << "In loadNoises() Query: " << query;

  int latest_update_time_ti = 1e100;
  int latest_update_time_ri = 1e100;
  int nID_ti=0;
  int nID_ri=0;
  
  while ( db->next() )
    {
      int       noiseID        = db->value( 0 ).toInt();
      QString   noiseType      = db->value( 1 ).toString();
      //QString  date                = US_Util::toUTCDatetimeText( db->value( 2 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
      QDateTime date          = db->value( 2 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();

      qDebug() << "Noises: noiseID, noiseType, date -- " << noiseID << noiseType << date; 
      
      if ( noiseType.contains( "ti_" ) ) 
	{
	  int time_to_now = date.secsTo(now);
	  if ( time_to_now < latest_update_time_ti )
	    {
	      latest_update_time_ti = time_to_now;
	      	      
	      nID_ti       = noiseID;
	    }
	}
      if ( noiseType.contains( "ri_" ) ) 
	{
	  int time_to_now = date.secsTo(now);
	  if ( time_to_now < latest_update_time_ri )
	    {
	      latest_update_time_ri = time_to_now;
	      	      
	      nID_ri       = noiseID;
	    }
	}
    }


  //ALEXEY: treat the case when model (like -MC does not possess its own noises -- use latest available noises for prior model like -IT  )
  //int US_LoadableNoise::count_noise() in ../../gui/us_loadable_noise.cpp
  //void US_FeMatch::load_noise( ) in us_fematch.cpp
  if ( !nID_ti && !nID_ri ) 
    loadNoises_whenAbsent();
  
  //creare US_noise objects
  if ( nID_ti )
    {
      ti_noise.load( QString::number( nID_ti ), db );
      qDebug() << "loadNoises() NORMAL: ti_noise created: ID -- " << nID_ti;
    }
  if ( nID_ri )
    {
      ri_noise.load( QString::number( nID_ri ), db );
      qDebug() << "loadNoises() NORMAL: ri_noise created: ID -- " << nID_ri;
    }
  
  // noise loaded:  insure that counts jive with data
  int ntinois = ti_noise.values.size();
  int nrinois = ri_noise.values.size();
  int nscans  = edata->scanCount();
  int npoints = edata->pointCount();
  int npadded = 0;


  qDebug() << "ti_noise.count, ri_noise.count: " <<  ti_noise.count << ri_noise.count;
  qDebug() << "ti_noise.values.size(), ri_noise.values.size(): " << ti_noise.values.size() << ri_noise.values.size();
  
  if ( ntinois > 0  &&  ntinois < npoints )
    {  // pad out ti noise values to radius count
      int jj      = ntinois;
      while ( jj++ < npoints )
	ti_noise.values << 0.0;
      ti_noise.count = ti_noise.values.size();
      npadded++;
    }
  
  if ( nrinois > 0  &&  nrinois < nscans )
    {  // pad out ri noise values to scan count
      int jj      = nrinois;
      while ( jj++ < nscans )
	ri_noise.values << 0.0;
      ri_noise.count = ri_noise.values.size();
      npadded++;
    }
  
  if ( npadded  > 0 )
      {  // let user know that padding occurred
	QString pmsg;
	
	if ( npadded == 1 )
	  pmsg = tr( "The noise file was padded out with zeroes\n"
		     "in order to match the data range." );
	else
	  pmsg = tr( "The noise files were padded out with zeroes\n"
		     "in order to match the data ranges." );
	
	QMessageBox::information( this, tr( "Noise Padded Out" ), pmsg );
      }

  return true;
}

//Load Noises when absent for the model loaded (like -MC models)
void US_ReporterGMP::loadNoises_whenAbsent( )
{
   QStringList mieGUIDs;  // list of GUIDs of models-in-edit
   QStringList nieGUIDs;  // list of GUIDS:type:index of noises-in-edit
   QString     editGUID  = edata->editGUID;         // loaded edit GUID
   QString     modelGUID = model.modelGUID;         // loaded model GUID
   
   int noisdf  = US_Settings::noise_dialog();
   int nenois  = count_noise_auto( edata, &model, mieGUIDs, nieGUIDs );

   qDebug() << "load_noise_whenAbsent(): mieGUIDs, nieGUIDs, editGUID, modelGUID -- " <<  mieGUIDs << nieGUIDs << editGUID << modelGUID;
   qDebug() << "load_noise_whenAbsent(): noisdf, nenois -- " << noisdf << nenois;

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      US_Passwd pw;
      US_DB2* dbP  = new US_DB2( pw.getPasswd() );

      if ( nenois > 1  &&  noisdf == 0 )
      {  // Noise exists and noise-dialog flag set to "Auto-load"
         QString descn = nieGUIDs.at( 0 );
         QString noiID = descn.section( ":", 0, 0 );
         QString typen = descn.section( ":", 1, 1 );
         QString mdlx1 = descn.section( ":", 2, 2 );

         if ( typen == "ti" )
	   {
	     ti_noise.load( true, noiID, dbP );
	     qDebug() << "load_noise_whenAbsent(): ti_noise created: ID -- " << noiID;
	     qDebug() << "load_noise_whenAbsent(): ti_noise.count: -- " << ti_noise.count;
	   }
         else
	   {
	     ri_noise.load( true, noiID, dbP );
	     qDebug() << "load_noise_whenAbsent(): ri_noise created: ID -- " << noiID;
	     qDebug() << "load_noise_whenAbsent(): ri_noise.count: -- " << ri_noise.count;
	   }
	 
         descn         = nieGUIDs.at( 1 );
         QString mdlx2 = descn.section( ":", 2, 2 );
         int kenois    = ( mdlx1 == mdlx2 ) ? 2 : 1;
         if ( kenois == 2 )
         {  // Second noise from same model:  g/et it, too
            noiID         = descn.section( ":", 0, 0 );
            typen         = descn.section( ":", 1, 1 );
            if ( typen == "ti" )
	      {
		ti_noise.load( true, noiID, dbP );
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ti--" << noiID;
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ti_noise.count --" << ti_noise.count;
	      }
            else
	      {
		ri_noise.load( true, noiID, dbP );
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ri--" << noiID;
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ri_noise.count --" << ri_noise.count;
	      }
	 }

      }
      else
      {  // only 1:  just load it
         QString noiID = nieGUIDs.at( 0 );
         QString typen = noiID.section( ":", 1, 1 );
         noiID         = noiID.section( ":", 0, 0 );

         if ( typen == "ti" )
	   ti_noise.load( true, noiID, dbP );
	 
         else
	   ri_noise.load( true, noiID, dbP );
      }
   }
}


// Determine if edit/model related noise available and build lists
int US_ReporterGMP::count_noise_auto( US_DataIO::EditedData* edata,
					US_Model* model, QStringList& mieGUIDs, QStringList& nieGUIDs  )
{
   int noidiag = US_Settings::noise_dialog();

   int nenois  = 0;       // number of edited-data-related noises
   
   if ( edata == NULL )
     return nenois;

   QStringList nimGUIDs;  // list of GUIDs:type:index of noises-in-models
   QStringList tmpGUIDs;  // temporary noises-in-model list
   QString     daEditGUID = edata->editGUID;        // loaded edit GUID
   QString     modelGUID  = ( model == 0 ) ?        // loaded model GUID
                           "" : model->modelGUID;
   QString     lmodlGUID;                           // list model GUID
   QString     lnoisGUID;                           // list noise GUID
   QString     modelIndx;                           // "0001" style model index

   id_list_db_auto  ( daEditGUID );

   // Get a list of models-with-noise tied to the loaded edit
   int nemods  = models_in_edit_auto ( daEditGUID, mieGUIDs );

   if ( nemods == 0 )
   {
     //QApplication::restoreOverrideCursor();
      return nemods;          // Go no further if no models-with-noise for edit
   }

   int latemx  = 0;   // Index to latest model-in-edit

   // If no model is loaded, pick the model GUID of the latest noise
   if ( model == 0 )
      modelGUID   = mieGUIDs[ latemx ];

   // Get a list of noises tied to the loaded model
   int nmnois  = noises_in_model_auto( modelGUID, nimGUIDs );

   // If the loaded model has no noise, try the latest model
   if ( nmnois == 0 )
   {
      modelGUID   = mieGUIDs[ latemx ];
      nmnois      = noises_in_model_auto( modelGUID, nimGUIDs );
   }

   // Insure that the loaded/latest model heads the model-in-edit list
   if ( modelGUID != mieGUIDs[ 0 ] )
   {
      if ( ! mieGUIDs.removeOne( modelGUID ) )
      {
         qDebug( "*ERROR* Loaded/Latest model not in model-in-edit list!" );
         QApplication::restoreOverrideCursor();
         return 0;
      }

      mieGUIDs.insert( 0, modelGUID );
   }


   int kk = 0;                // running output models index

   if ( nmnois > 0 )
   {  // If loaded model has noise, put noise in list
      nieGUIDs << nimGUIDs;   // initialize noise-in-edit list
      kk++;
   }

   nenois      = nmnois;      // initial noise-in-edit count is noises in model

   for ( int ii = 1; ii < nemods; ii++ )
   {  // Search through models in edit
      lmodlGUID  = mieGUIDs[ ii ];                    // this model's GUID
      modelIndx  = QString().sprintf( "%4.4d", kk );  // models-in-edit index

      // Find the noises tied to this model
      int kenois = noises_in_model_auto( lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      {  // if we have 1 or 2 noises, add to noise-in-edit list
         nenois    += qMin( 2, kenois );
         // adjust entry to have the right model-in-edit index
         lnoisGUID  = tmpGUIDs.at( 0 ).section( ":", 0, 1 )
            + ":" + modelIndx;
         nieGUIDs << lnoisGUID;
         if ( kenois > 1 )
         {  // add a second noise to the list
            lnoisGUID  = tmpGUIDs.at( 1 ).section( ":", 0, 1 )
               + ":" + modelIndx;
            nieGUIDs << lnoisGUID;
         }
	 
         kk++;
      }
   }


   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     amsg;
      QString     msg;

      if ( model == 0 )
         amsg = tr( ", associated with the loaded edit.\n" );

      else
         amsg = tr( ", associated with the loaded edit/model.\n" );

      if ( nenois > 1 )
      {
         msg  = tr( "There are noise files" ) + amsg
              + tr( "Do you want to load some of them?" );
      }

      else
      {  // Single noise file: check its value range versus experiment
         QString noiID  = nieGUIDs.at( 0 ).section( ":", 0, 0 );
         US_Noise i_noise;

	 US_Passwd pw;
	 US_DB2 db( pw.getPasswd() );
	 i_noise.load( true, noiID, &db );
         
         double datmin  = edata->value( 0, 0 );
         double datmax  = datmin;
         double noimin  = 1.0e10;
         double noimax  = -noimin;
         int    npoint  = edata->pointCount();

         for ( int ii = 0; ii < edata->scanData.size(); ii++ )
         {
            for ( int jj = 0; jj < npoint; jj++ )
            {
               double datval = edata->value( ii, jj );
               datmin        = qMin( datmin, datval );
               datmax        = qMax( datmax, datval );
            }
         }

         for ( int ii = 0; ii < i_noise.values.size(); ii++ )
         {
            double noival = i_noise.values[ ii ];
            noimin        = qMin( noimin, noival );
            noimax        = qMax( noimax, noival );
         }

         if ( ( noimax - noimin ) > ( datmax - datmin ) )
         {  // Insert a warning if noise appears corrupt or unusual
            amsg = amsg
               + tr( "\nBUT THE NOISE HAS AN UNUSUALLY LARGE DATA RANGE.\n\n" );
         }

         msg  = tr( "There is a noise file" ) + amsg
              + tr( "Do you want to load it?" );
      }

DbgLv(2) << "LaNoi:noidiag  " << noidiag;
      if ( noidiag > 0 )
      {
         msgBox.setWindowTitle( tr( "Edit/Model Associated Noise" ) );
         msgBox.setText( msg );
         msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() != QMessageBox::Yes )
         {  // user did not say "yes":  return zero count
            nenois  = 0;       // number of edited-data-related noises
         }
      }

      if ( kk < nemods )
      {  // Models with noise were found, so truncate models list
         for ( int ii = 0; ii < ( nemods - kk ); ii++ )
            mieGUIDs.removeLast();
      }
   }

   return nenois;
}

// build a list of noise(GUIDs) for a given model(GUID)
int US_ReporterGMP::noises_in_model_auto( QString mGUID, QStringList& nGUIDs )
{
   QString xnGUID;
   QString xmGUID;
   QString xntype;

   nGUIDs.clear();

   for ( int ii = 0; ii < noiIDs.size(); ii++ )
   {  // Examine noises list; Save to this list if model GUID matches
      xnGUID = noiIDs  .at( ii );
      xmGUID = noiMoIDs.at( ii );
      xntype = noiTypes.at( ii );

      xntype = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ?
	"ri" : "ti";
     
      if ( mGUID == xmGUID )
	nGUIDs << xnGUID + ":" + xntype + ":0000";
   }
   
   return nGUIDs.size();
}

// Build a list of models(GUIDs) for a given edit(GUID)
int US_ReporterGMP::models_in_edit_auto( QString eGUID, QStringList& mGUIDs )
{
   QString xmGUID;
   QString xeGUID;
   QString xrGUID;
   QStringList reGUIDs;

   mGUIDs.clear();

   for ( int ii = 0; ii < modIDs.size(); ii++ )
   {  // Examine models list; Save to this list if edit GUID matches
      xmGUID = modIDs.at( ii );
      xeGUID = modEdIDs.at( ii );
     
      if ( eGUID == xeGUID )
      {
         mGUIDs << xmGUID;
      }
   }

   return mGUIDs.size();
}

// Build lists of noise and model IDs for database
int US_ReporterGMP::id_list_db_auto( QString daEditGUID )
{
   QStringList query;
   
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
      return 0;

   query.clear();
   query << "get_editID" << daEditGUID;
   db.query( query );
   db.next();
   QString daEditID = db.value( 0 ).toString();
DbgLv(1) << "LaNoi:idlDB:  daEdit ID GUID" << daEditID << daEditGUID;

   noiIDs  .clear();
   noiEdIDs.clear();
   noiMoIDs.clear();
   noiTypes.clear();
   modIDs  .clear();
   modEdIDs.clear();
   modDescs.clear();

   QStringList reqIDs;
   QString     noiEdID;

   // Build noise, edit, model ID lists for all noises
   query.clear();
   query << "get_noise_desc_by_editID" << QString::number( invID ) << daEditID;
   db.query( query );

   while ( db.next() )
   {  // Accumulate lists from noise records
      noiEdID   = db.value( 2 ).toString();

      noiIDs   << db.value( 1 ).toString();
      noiTypes << db.value( 4 ).toString();
      noiMoIDs << db.value( 5 ).toString();
   }

DbgLv(1) << "LaNoi:idlDB: noiTypes size" << noiTypes.size();
   // Build model, edit ID lists for all models
   query.clear();
   query << "get_model_desc_by_editID" << QString::number( invID ) << daEditID;
   db.query( query );

   while ( db.next() )
   {  // Accumulate from db desc entries matching noise model IDs
      QString modGUID = db.value( 1 ).toString();
      QString modEdID = db.value( 6 ).toString();

      if ( noiMoIDs.contains( modGUID )  &&   modEdID == daEditID )
      {  // Only list models that have associated noise and match edit
         modIDs   << modGUID;
         modDescs << db.value( 2 ).toString();
         modEdIDs << db.value( 5 ).toString();
      }
   }
DbgLv(1) << "LaNoi:idlDB: modDescs size" << modDescs.size();

   // Loop through models to edit out any extra monteCarlo models
   for ( int ii = modIDs.size() - 1; ii >=0; ii-- )
   {  // Work from the back so any removed records do not affect indexes
      QString mdesc  = modDescs.at( ii );
      QString asysID = mdesc.section( ".", -2, -2 );
      bool    mCarlo = ( asysID.contains( "-MC" )  &&
                         asysID.contains( "_mc" ) );
      QString reqID  = asysID.section( "_", 0, -2 );

      if ( mCarlo )
      {  // Treat monte carlo in a special way (as single composite model)
         if ( reqIDs.contains( reqID ) )
         {  // already have this request GUID, so remove this model
            modIDs  .removeAt( ii );
            modDescs.removeAt( ii );
            modEdIDs.removeAt( ii );
         }

         else
         {  // This is the first time for this request, so save it in a list
            reqIDs << reqID;
         }
      }
   }

   // Create list of edit GUIDs for noises
   for ( int ii = 0; ii < noiTypes.size(); ii++ )
   {
      QString moGUID  = noiMoIDs.at( ii );
      int     jj      = modIDs.indexOf( moGUID );
DbgLv(2) << "LaNoi:idlDB: ii jj moGUID" << ii << jj << moGUID;

      QString edGUID  = ( jj < 0 ) ? "" : modEdIDs.at( jj );

      noiEdIDs << edGUID;
   }

   return noiIDs.size();
}

//Simulate Model
void US_ReporterGMP::simulateModel( QMap < QString, QString > & tripleInfo )
{
  progress_msg->setLabelText( QString("Simulating model %1 for triple %2...")
			      .arg( tripleInfo[ "stage_name" ])
			      .arg( tripleInfo[ "triple_name" ] ) );
  progress_msg->setValue( 0 );
  qApp->processEvents();
  
  int    nconc   = edata->pointCount();
  double radlo   = edata->radius( 0 );
  double radhi   = edata->radius( nconc - 1 );

  int lc=model_used.components.size()-1;
  qDebug() << "SimMdl: 0) s D c"
	   << model_used.components[ 0].s << model_used.components[ 0].D
	   << model_used.components[ 0].signal_concentration << "  n" << lc;
  qDebug() << "SimMdl: n) s D c"
	   << model_used.components[lc].s << model_used.components[lc].D
	   << model_used.components[lc].signal_concentration;

  adjustModel();

  qDebug() << "SimMdl: 0) s D c"
	   << model.components[ 0].s << model.components[ 0].D
	   << model.components[ 0].signal_concentration;
  qDebug() << "SimMdl: n) s D c"
	   << model.components[lc].s << model.components[lc].D
	   << model.components[lc].signal_concentration;
   
  // Initialize simulation parameters using edited data information
  US_Passwd pw;
  US_DB2* dbP = new US_DB2( pw.getPasswd() );
  
  simparams.initFromData( dbP, *edata, dat_steps );
  simparams.simpoints         = adv_vals[ "simpoints" ].toInt();
  simparams.meshType          = US_SimulationParameters::ASTFEM;
  simparams.gridType          = US_SimulationParameters::MOVING;
  simparams.radial_resolution = (double)( radhi - radlo ) / ( nconc - 1 );
  //   simparams.bottom            = simparams.bottom_position;
  qDebug() << "SimMdl: simpoints" << simparams.simpoints
	   << "rreso" << simparams.radial_resolution
	   << "bottom_sim" << simparams.bottom << "bottom_dat" << edata->bottom;
  //simparams.meniscus          = 5.8;
  
  //sdata.scanData.resize( total_scans );
  //int terpsize    = ( points + 7 ) / 8;
  
  if ( exp_steps )
    simparams.speed_step        = speed_steps;
  
  qDebug() << "SimMdl: speed_steps:" << simparams.speed_step.size();
  
  QString mtyp = adv_vals[ "meshtype"  ];
  QString gtyp = adv_vals[ "gridtype"  ];
  QString bvol = adv_vals[ "bndvolume" ];
  
  
  if ( gtyp.contains( "Constant" ) )
    simparams.gridType = US_SimulationParameters::FIXED;
  
  if ( model.components[ 0 ].sigma == 0.0  &&
       model.components[ 0 ].delta == 0.0)
    simparams.meshType = US_SimulationParameters::ASTFEM;
  else
    simparams.meshType = US_SimulationParameters::ASTFVM;
  
  simparams.firstScanIsConcentration = false;
  
  double concval1      = 0.0;
  
  if ( simparams.band_forming )
    {
      simparams.band_volume = bvol.toDouble();
      //concval1              = 1.0;
      //simparams.firstScanIsConcentration = true;
    }
  else
    simparams.band_volume = 0.0;
  
  // Make a simulation copy of the experimental data without actual readings

  qDebug() << "SimulateModel: --- 1";
  
  US_AstfemMath::initSimData( *sdata, *edata, concval1 );// Gets experimental time grid set

  qDebug() << "SimulateModel: --- 2";
  
  
  QString tmst_fpath = US_Settings::resultDir() + "/" + FileName_parsed + "/"
    + FileName_parsed + ".time_state.tmst";
  QFileInfo check_file( tmst_fpath );
  simparams.sim      = ( edata->channel == "S" );
  
  if ( ( check_file.exists() ) && ( check_file.isFile() ) )
    {
      if ( US_AstfemMath::timestate_onesec( tmst_fpath, *sdata ) )
	{  // Load timestate that is already at 1-second-intervals
	  simparams.simSpeedsFromTimeState( tmst_fpath );
	  qDebug() << "SimMdl: timestate file exists" << tmst_fpath << " timestateobject,count = "
		   << simparams.tsobj << simparams.sim_speed_prof.count();
	  simparams.speedstepsFromSSprof();
	}
      
      else
	{  // Replace timestate with a new one that is at 1-second-intervals
	  // if ( drow == 0 )
	  //   {
	  QString tmst_fdefs = QString( tmst_fpath ).replace( ".tmst", ".xml" );
	  QString tmst_fpsav = tmst_fpath + "-orig";
	  QString tmst_fdsav = tmst_fdefs + "-orig";
	  simparams.sim      = ( edata->channel == "S" );
	  
	  // Rename existing (non-1sec-intv) files
	  QFile::rename( tmst_fpath, tmst_fpsav );
	  QFile::rename( tmst_fdefs, tmst_fdsav );
	  // Create a new 1-second-interval file set
	  US_AstfemMath::writetimestate( tmst_fpath, simparams, *sdata );
	  qDebug() << "SimMdl: 1-sec-intv file created";
	  // }
	  
	  simparams.simSpeedsFromTimeState( tmst_fpath );
	  simparams.speedstepsFromSSprof();
	}
    }
  else
    {
      qDebug() << "SimMdl: timestate file does not exist";
      if ( ! simparams.sim )
	{  // Compute acceleration rate for non-astfem_sim data
	  const double tfac = ( 4.0 / 3.0 );
	  double t2   = simparams.speed_step[ 0 ].time_first;
	  double w2t  = simparams.speed_step[ 0 ].w2t_first;
	  double om1t = simparams.speed_step[ 0 ].rotorspeed * M_PI / 30.0;
	  double w2   = sq( om1t );
	  double t1   = tfac * ( t2 - ( w2t / w2 ) );
	  int t_acc   = (int)qRound( t1 );
	  double rate = (double)( simparams.speed_step[ 0 ].rotorspeed )
	    / (double)t_acc;
	  qDebug() << "SimMdl:  accel-calc:  t1 t2 w2t t_acc speed rate"
		   << t1 << t2 << w2t << t_acc << simparams.speed_step[0].rotorspeed << rate;
	  simparams.speed_step[ 0 ].acceleration = (int)qRound( rate );
	}
    }
  
  // Do a quick test of the speed step implied by TimeState
  int tf_scan   = simparams.speed_step[ 0 ].time_first;
  int accel1    = simparams.speed_step[ 0 ].acceleration;
  QString svalu = US_Settings::debug_value( "SetSpeedLowA" );
  int lo_ss_acc = svalu.isEmpty() ? 250 : svalu.toInt();
  int rspeed    = simparams.speed_step[ 0 ].rotorspeed;
  int tf_aend   = ( rspeed + accel1 - 1 ) / accel1;
  
  qDebug() << "SimMdl: ssck: rspeed accel1 lo_ss_acc"
	   << rspeed << accel1 << lo_ss_acc << "tf_aend tf_scan"
	   << tf_aend << tf_scan;
  //x0  1  2  3  4  5
  if ( accel1 < lo_ss_acc  ||  tf_aend > ( tf_scan - 3 ) )
    {
      QString wmsg = tr( "The TimeState computed/used is likely bad:<br/>"
			 "The acceleration implied is %1 rpm/sec.<br/>"
			 "The acceleration zone ends at %2 seconds,<br/>"
			 "with a first scan time of %3 seconds.<br/><br/>"
			 "<b>You should rerun the experiment without<br/>"
			 "any interim constant speed, and then<br/>"
			 "you should reimport the data.</b>" )
	.arg( accel1 ).arg( tf_aend ).arg( tf_scan );
      
      QMessageBox msgBox( this );
      msgBox.setWindowTitle( tr( "Bad TimeState Implied!" ) );
      msgBox.setTextFormat( Qt::RichText );
      msgBox.setText( wmsg );
      msgBox.addButton( tr( "Continue" ), QMessageBox::RejectRole );
      QPushButton* bAbort = msgBox.addButton( tr( "Abort" ),
					      QMessageBox::YesRole    );
      msgBox.setDefaultButton( bAbort );
      msgBox.exec();
      if ( msgBox.clickedButton() == bAbort )
	{
	  QApplication::restoreOverrideCursor();
	  qApp->processEvents();
	  return;
	}
    }
  sdata->cell        = rdata->cell;
  sdata->channel     = rdata->channel;
  sdata->description = rdata->description;
  
  if ( dbg_level > 0 )
    simparams.save_simparms( US_Settings::etcDir() + "/sp_fematch.xml" );
  
  //start_time = QDateTime::currentDateTime();
  int ncomp  = model.components.size();
  //compress   = le_compress->text().toDouble();
  progress_msg->setRange( 1, ncomp );
  // progress_msg->reset();
  
  nthread    = US_Settings::threads();
  int ntc    = ( ncomp + nthread - 1 ) / nthread;
  nthread    = ( ntc > MIN_NTC ) ? nthread : 1;

  /*
  //TEST
  nthread = 10;
  */
  
  qDebug() << "SimMdl: nthread" << nthread << "ncomp" << ncomp
	   << "ntc" << ntc << "meshtype" << simparams.meshType;
  
  // Do simulation by several possible ways: 1-/Multi-thread, ASTFEM/ASTFVM
  //if ( nthread < 2 )
  if ( nthread < 999 )
    {
      if ( model.components[ 0 ].sigma == 0.0  &&
	   model.components[ 0 ].delta == 0.0  &&
	   model.coSedSolute           <  0.0  &&
	   compress                    == 0.0 )
	{
	  qDebug() << "SimMdl: (fematch:)Finite Element Solver is called";
	  //*DEBUG*
	  for(int ii=0; ii<model.components.size(); ii++ )
	    {
	      qDebug() << "SimMdl:   comp" << ii << "s D v"
		       << model.components[ii].s
		       << model.components[ii].D
		       << model.components[ii].vbar20 << "  c"
		       << model.components[ii].signal_concentration;
	    }
	  qDebug() << "SimMdl: (fematch:)Sim Pars--";
	  simparams.debug();
	  //*DEBUG*
	  US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
	  
	  connect( astfem_rsa, SIGNAL( current_component( int ) ),
	   	   this,       SLOT  ( update_progress  ( int ) ) );
	  astfem_rsa->set_debug_flag( dbg_level );
	  solution_rec.buffer.compressibility = compress;
	  solution_rec.buffer.manual          = manual;
	  //astfem_rsa->set_buffer( solution_rec.buffer );
	  
	  astfem_rsa->calculate( *sdata );
	  //*DEBUG*
	  int kpts=0;
	  double trmsd=0.0;
	  double tnoi=0.0;
	  double rnoi=0.0;
	  bool ftin=ti_noise.count > 0;
	  bool frin=ri_noise.count > 0;
	  for(int ss=0; ss<sdata->scanCount(); ss++)
	    {
	      rnoi = frin ? ri_noise.values[ss] : 0.0;
	      for (int rr=0; rr<sdata->pointCount(); rr++)
		{
		  tnoi = ftin ? ti_noise.values[rr] : 0.0;
		  double rval=edata->value(ss,rr) - sdata->value(ss,rr) - rnoi - tnoi;
		  trmsd += sq( rval );
		  kpts++;
		}
	    }
	  trmsd = sqrt( trmsd / (float)kpts );
	  qDebug() << "SimMdl: (1)trmsd" << trmsd;
	  //*DEBUG*
	}
      else
	{
	  qDebug() << "SimMdl: (fematch:)Finite Volume Solver is called";
	  US_LammAstfvm *astfvm     = new US_LammAstfvm( model, simparams );
	  astfvm->calculate(     *sdata );
	}
      //-----------------------
      //Simulation part is over
      //-----------------------
      
      show_results();
    }
  
  else
    {  // Do multi-thread calculations

      qDebug() << "Simulate Model: Multi-thread -- ";
      
      solution_rec.buffer.compressibility = compress;
      solution_rec.buffer.manual          = manual;
      tsimdats.clear();
      tmodels .clear();
      kcomps  .clear();
      QList< ThreadWorker* >         tworkers;
      QList< QThreadEx* >            wthreads;
      
      // Build models for each thread
      for ( int ii = 0; ii < ncomp; ii++ )
	{
	  if ( ii < nthread )
	    {  // First time through per thread:  get initial model and sim data
	      tmodels << model;
	      tmodels[ ii ].components.clear();
	      US_DataIO::RawData sdat = *sdata;
	      tsimdats << sdat;
	      kcomps   << 0;
	    }
	  
	  // Partition thread models from round-robin fetch of components
	  int jj = ii % nthread;
	  tmodels[ jj ].components << model.components[ ii ];
	}
      
      thrdone   = 0;
      solution_rec.buffer.manual = manual;
      
      // Build worker threads and begin running
      for ( int ii = 0; ii < nthread; ii++ )
	{
	  ThreadWorker* tworker = new ThreadWorker( tmodels[ ii ], simparams,
						    tsimdats[ ii ], solution_rec.buffer, ii );
	  QThreadEx*    wthread = new QThreadEx();
	  
	  tworker->moveToThread( wthread );
	  
	  tworkers << tworker;
	  wthreads << wthread;
	  
	  connect( wthread, SIGNAL( started()         ),
		   tworker, SLOT  ( calc_simulation() ) );
	  
	  connect( tworker, SIGNAL( work_progress  ( int, int ) ),
	   	   this,    SLOT(   thread_progress( int, int ) ) );
	  connect( tworker, SIGNAL( work_complete  ( int )      ),
		   this,    SLOT(   thread_complete( int )      ) );
	  
	  wthread->start();
	}
    }
}

// Update progress bar as each component is completed
void US_ReporterGMP::update_progress( int icomp )
{
  qDebug () << "Updating progress single thread, icomp  -- " << icomp;
  
  progress_msg->setValue( icomp );
}


// Show simulation and residual when the simulation is complete
void US_ReporterGMP::show_results( )
{
   progress_msg->setValue( progress_msg->maximum() );
   qApp->processEvents();
   
   haveSim     = true;
   calc_residuals();             // calculate residuals

   //distrib_plot_resids();        // plot residuals

   // data_plot();                  // re-plot data+simulation

   // if ( rbmapd != 0 )
   // {
   //    bmd_pos  = rbmapd->pos();
   //    rbmapd->close();
   // }

   assemble_distrib_html( );
   
   plotres(); // <------- save plots into files locally
   
   
   QApplication::restoreOverrideCursor();
}

// Calculate residual absorbance values (data - sim - noise)
void US_ReporterGMP::calc_residuals()
{
   int     dsize  = edata->pointCount();
   int     ssize  = sdata->pointCount();
   QVector< double > vecxx( dsize );
   QVector< double > vecsx( ssize );
   QVector< double > vecsy( ssize );
   double* xx     = vecxx.data();
   double* sx     = vecsx.data();
   double* sy     = vecsy.data();
   double  yval;
   double  sval;
   double  rmsd   = 0.0;
   double  tnoi   = 0.0;
   double  rnoi   = 0.0;
   bool    ftin   = ti_noise.count > 0;
   bool    frin   = ri_noise.count > 0;
   bool    matchd = ( dsize == ssize );
   int     kpts   = 0;
   qDebug() << "CALC_RESID: matchd" << matchd << "dsize ssize" << dsize << ssize;
   int kexcls=0;
 
   QVector< double > resscan;

   resids .clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[ jj ] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[ jj ] = sdata->radius( jj );
      if ( sx[ jj ] != xx[ jj ] )  matchd = false;
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excludedScans.contains( ii );
if(!usescan) kexcls++;

      rnoi     = frin ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[ jj ] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      { // Calculate the residuals and the RMSD
         tnoi          = ftin ? ti_noise.values[ jj ] : 0.0;

         if ( matchd )
            sval          = sy[ jj ];
         else
            sval          = interp_sval( xx[ jj ], sx, sy, ssize );

         yval          = edata->value( ii, jj ) - sval - rnoi - tnoi;

         if ( usescan )
         {
            rmsd         += sq( yval );
            kpts++;
         }

         resscan[ jj ] = yval;
      }

      resids << resscan;
   }

   rmsd  /= (double)( kpts );
   //le_variance->setText( QString::number( rmsd ) );
   rmsd   = sqrt( rmsd );

   rmsd_global =  QString::number( rmsd );

   qDebug() << "CALC_RESID: matchd" << matchd << "kexcls" << kexcls << "rmsd" << rmsd;

}


//output HTML string for Distributions for current triple:
void  US_ReporterGMP::assemble_distrib_html( void )
{
  //QString html_distibutions = distrib_info();
  html_assembled += "<p class=\"pagebreak \">\n";
  html_assembled += html_header( "US_Fematch", text_model( model, 2 ), edata );
  html_assembled += distrib_info();
  html_assembled += "</p>\n";
}

//output HTML plots for currentTriple
void  US_ReporterGMP::assemble_plots_html( QStringList PlotsFilenames )
{
  // Embed plots in the composite report
  for ( int i = 0;  i < PlotsFilenames.size(); ++ i )
    {
      QString filename = PlotsFilenames[ i ];
      QString label = "";
      
      html_assembled   += "    <div><img src=\"" + filename 
	+ "\" alt=\"" + label + "\"/></div>\n\n";
    }
}
  
// Interpolate an sdata y (readings) value for a given x (radius)
double US_ReporterGMP::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[ jj ] )
      {
         return sy[ jj ];
      }

      if ( xv < sx[ jj ] )
      {  // given x lower than array x: interpolate between point and previous
         int    ii = jj - 1;
         double dx = sx[ jj ] - sx[ ii ];
         double dy = sy[ jj ] - sy[ ii ];
         return ( sy[ ii ] + ( xv - sx[ ii ] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[ jj ] - sx[ ii ];
   double dy = sy[ jj ] - sy[ ii ];
   return ( sy[ ii ] + ( xv - sx[ ii ] ) * dy / dx );
}

// Model type text string
QString US_ReporterGMP::text_model( US_Model model, int width )
{
   QString stitle = model.typeText();
   QString title  = stitle;

   if ( width != 0 )
   {  // long title:  add any suffixes and check need to center
      switch ( (int)model.analysis )
      {
         case (int)US_Model::TWODSA:
         case (int)US_Model::TWODSA_MW:
            title = tr( "2-Dimensional Spectrum Analysis" );
            break;

         case (int)US_Model::GA:
         case (int)US_Model::GA_MW:
            title = tr( "Genetic Algorithm Analysis" );
            break;

         case (int)US_Model::COFS:
            title = tr( "C(s) Analysis" );
            break;

         case (int)US_Model::FE:
            title = tr( "Finite Element Analysis" );
            break;

         case (int)US_Model::PCSA:
            title = tr( "Parametrically Constrained Spectrum Analysis\n" );

            if ( stitle.contains( "-SL" ) )
               title += tr( "(Straight Line)" );

            else if ( stitle.contains( "-IS" ) )
               title += tr( "(Increasing Sigmoid)" );

            else if ( stitle.contains( "-DS" ) )
               title += tr( "(Decreasing Sigmoid)" );

            else if ( stitle.contains( "-HL" ) )
               title += tr( "(Horizontal Line)" );

            else if ( stitle.contains( "-2O" ) )
               title += tr( "(2nd-Order Power Law)" );

            break;

         case (int)US_Model::DMGA:
            title = tr( "Discrete Model Genetic Algorithm" );
            break;

         case (int)US_Model::MANUAL:
         default:
            title = tr( "2-Dimensional Spectrum Analysis" );
            break;
      }


            if ( stitle.contains( "-SL" ) )
      if ( model.associations.size() > 1 )
         title = title + " (RA)";

      if ( model.global == US_Model::MENISCUS )
         title = title + " (Menisc.)";

      else if ( model.global == US_Model::GLOBAL )
         title = title + " (Global)";

      else if ( model.global == US_Model::SUPERGLOBAL )
         title = title + " (S.Glob.)";

      if ( model.monteCarlo )
         title = title + " (MC)";

      if ( width > title.length() )
      {  // long title centered:  center it in fixed-length string
         int lent = title.length();
         int lenl = ( width - lent ) / 2;
         int lenr = width - lent - lenl;
         title    = QString( " " ).repeated( lenl ) + title
                  + QString( " " ).repeated( lenr );
      }
   }

   return title;
}


// Compose a report HTML header
QString US_ReporterGMP::html_header( QString title, QString head1,
				     US_DataIO::EditedData* edata )
{
   QString s = QString( "<?xml version=\"1.0\"?>\n" );
   s  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   s  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
         "/xhtml1-strict.dtd\">\n";
   s  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
         " xml:lang=\"en\" lang=\"en\">\n";
   s  += "  <head>\n";
   s  += "    <title> " + title + " </title>\n";
   s  += "    <meta http-equiv=\"Content-Type\" content="
         "\"text/html; charset=iso-8859-1\"/>\n";
   s  += "    <style type=\"text/css\" >\n";
   s  += "      td { padding-right: 1em; }\n";
   s  += "      body { background-color: white; }\n";

   s  += "    .pagebreak\n";
   s  += "    {\n";
   s  += "      page-break-before: always; border: 1px solid; \n";
   s  += "    }\n";
   
   s  += "    </style>\n";
   s  += "  </head>\n  <body>\n";
   s  += "    <h1>" + head1 + "</h1>\n";
   s  += indent( 2 ) + tr( "<h2>Data Report for Run \"" ) + edata->runID;
   s  += "\",<br/>\n" + indent( 2 ) + "&nbsp;" + tr( " Cell " ) + edata->cell;
   s  += tr( ", Channel " ) + edata->channel;
   s  += tr( ", Wavelength " ) + edata->wavelength;
   s  += ",<br/>\n" + indent( 2 ) + "&nbsp;" + tr( " Edited Dataset " );
   s  += edata->editID + "</h2>\n";

   return s;
}

// Distribution information HTML string
QString US_ReporterGMP::distrib_info()
{
   int  ncomp     = model_used.components.size();
   double vari_m  = model_used.variance;
   double rmsd_m  = ( vari_m == 0.0 ) ? 0.0 : sqrt( vari_m );

   if ( ncomp == 0 )
      return "";

   QString msim   = adv_vals[ "modelsim" ];

   if ( is_dmga_mc )
   {

      if ( msim == "model" )
      {  // Use DMGA-MC single-iteration model
         msim           = "<b>&nbsp;&nbsp;( single iteration )</b>";
      }
      else
      {  // Use mean|median|mode model
         msim           = "<b>&nbsp;&nbsp;( " + msim + " )</b>";
      }
   }
   else
   {  // Normal non-DMGA-MC model
      msim           = "";
      if ( model_used.monteCarlo  &&
           ! model_used.description.contains( "_mcN" ) )
         msim           = "<b>&nbsp;&nbsp;( single iteration )</b>";
   }

   QString mdla = model_used.description
                  .section( ".", -2, -2 ).section( "_", 1, -1 );
   if ( mdla.isEmpty() )
      mdla         = model_used.description.section( ".", 0, -2 );

   QString mstr = "\n" + indent( 2 )
                  + tr( "<h3>Data Analysis Settings:</h3>\n" )
                  + indent( 2 ) + "<table>\n";

   mstr += table_row( tr( "Model Analysis:" ), mdla + msim );
   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      rmsd_global  );
   mstr += table_row( tr( "Model-reported RMSD:"    ),
                      ( rmsd_m > 0.0 ) ? QString::number( rmsd_m ) : "(none)" );

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_c   = 0.0;
   double sum_v   = 0.0;
   double sum_k   = 0.0;
   double mink    = 1e+99;
   double maxk    = -1e+99;
   double minv    = 1e+99;
   double maxv    = -1e+99;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model_used.components[ ii ].signal_concentration;
      double kval = model_used.components[ ii ].f_f0;
      double vval = model_used.components[ ii ].vbar20;
      sum_c      += conc;
      sum_mw     += ( model_used.components[ ii ].mw * conc );
      sum_s      += ( model_used.components[ ii ].s  * conc );
      sum_D      += ( model_used.components[ ii ].D  * conc );
      sum_v      += ( vval * conc );
      sum_k      += ( kval * conc );
      mink        = qMin( kval, mink );
      maxk        = qMax( kval, maxk );
      minv        = qMin( vval, minv );
      maxv        = qMax( vval, maxv );
   }

   mstr += table_row( tr( "Weight Average s20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString().sprintf( "%6.4e", ( sum_mw / sum_c ) ) );
   if ( ! cnstff )
      mstr += table_row( tr( "Weight Average f/f0:" ),
                         QString::number( ( sum_k / sum_c ) ) );
   if ( ! cnstvb )
      mstr += table_row( tr( "Weight Average vbar20:" ),
                         QString::number( ( sum_v / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString().sprintf( "%6.4e", sum_c ) );

   if ( cnstvb )
      mstr += table_row( tr( "Constant vbar20:" ),
                         QString::number( minv ) );
   else if ( cnstff )
      mstr += table_row( tr( "Constant f/f0:" ),
                         QString::number( mink ) );
   mstr += indent( 2 ) + "</table>\n";

   mstr += "\n" + indent( 2 ) + tr( "<h3>Distribution Information:</h3>\n" );
   mstr += indent( 2 ) + "<table>\n";

   if ( cnstvb )
   {  // Normal constant-vbar distribution
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D Apparent" ),
                         tr( "D 20,W" ),     tr( "f / f0" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].mw   ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s    ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].s    ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].D    ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].D    ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].f_f0 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   else if ( cnstff )
   {  // Constant-f/f0, varying vbar
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D Apparent" ),
                         tr( "D 20,W" ),     tr( "Vbar20" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].mw     ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].vbar20 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   else
   {  // Neither vbar nor f/f0 are constant
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D 20,W"     ),
                         tr( "f / f0" ),     tr( "Vbar20" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].mw     ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                 model_used.components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].f_f0 ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].vbar20 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   // Show associations information if present
   if ( model_used.associations.size() > 0 )
   {
      mstr += indent( 2 ) + "</table>\n" + indent( 2 );
      mstr += tr( "<h3>Reversible Associations Information:</h3>\n" );
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Reactant 1" ), tr( "Reactant 2" ),
                         tr( "Product"    ), tr( "K_dissociation"  ),
                         tr( "k_off Rate" ) );

      for ( int ii = 0; ii < model_used.associations.size(); ii++ )
      {
         US_Model::Association as1 = model.associations[ ii ];
         double k_d    = as1.k_d;
         double k_off  = as1.k_off;
         QString reac1 = tr( "component %1" ).arg( as1.rcomps[ 0 ] + 1 );
         QString reac2 = tr( "(none)" );
         QString prod  = tr( "component %1" ).arg( as1.rcomps[ 1 ] + 1 );
         if ( as1.rcomps.size() > 2 )
         {
            reac2         = prod;
            prod          = tr( "component %1" ).arg( as1.rcomps[ 2 ] + 1 );
         }

         mstr       += table_row( reac1, reac2, prod,
                                  QString().sprintf( "%10.4e", k_d   ),
                                  QString().sprintf( "%10.4e", k_off ) );
      }
   }

   mstr += indent( 2 ) + "</table>\n";

   /*
   if ( is_dmga_mc )
   {  // Compute DMGA-MC statistics and add them to the report
      QVector< double >             rstats;
      QVector< QVector< double > >  mstats;
      int niters     = imodels.size();
      int ncomp      = imodels[ 0 ].components  .size();
      int nreac      = imodels[ 0 ].associations.size();

      // Build RMSD statistics across iterations
      US_DmgaMcStats::build_rmsd_stats( niters, imodels, rstats );

      // Build statistics across iterations
      int ntatt = US_DmgaMcStats::build_model_stats( niters, imodels, mstats );

      // Compose the summary statistics chart
      mstr += indent( 2 );
      mstr += tr( "<h3>Discrete Model GA-MC Summary Statistics:</h3>\n" );
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Component" ), tr( "Attribute" ),
                         tr( "Mean_Value" ), tr( "95%_Confidence(low)"  ),
                         tr( "95%_Confidence(high)" ) );
      int kd         = 0;
      QString fixd   = tr( "(Fixed)" );
      QString blnk( "" );
      QStringList atitl;
      QStringList rtitl;
      atitl << tr( "Concentration" )
            << tr( "Vbar20" )
            << tr( "Molecular Weight" )
            << tr( "Sedimentation Coefficient" )
            << tr( "Diffusion Coefficient" )
            << tr( "Frictional Ratio" );
      rtitl << tr( "K_dissociation" )
            << tr( "K_off Rate" );

      // Show summary of RMSDs
      mstr += table_row( tr( "(All)" ), tr( "RMSD" ),
                         QString().sprintf( "%10.4e", rstats[  2 ] ),
                         QString().sprintf( "%10.4e", rstats[  9 ] ),
                         QString().sprintf( "%10.4e", rstats[ 10 ] ) );

      // Show summary of component attributes
      for ( int ii = 0; ii < ncomp; ii++ )
      {
         QString compnum = QString().sprintf( "%2d", ii + 1 );
         for ( int jj = 0; jj < 6; jj++ )
         {
            bool is_fixed   = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );
            QString strclo  = is_fixed ? fixd :
                              QString().sprintf( "%10.4e", mstats[ kd ][  9 ] );
            QString strchi  = is_fixed ? blnk :
                              QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] );
            mstr += table_row( compnum, atitl[ jj ],
                              QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ),
                              strclo, strchi );
            kd++;
         }
      }

      mstr += indent( 2 ) + "</table>\n";
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Reaction" ), tr( "Attribute" ),
                         tr( "Mean_Value" ), tr( "95%_Confidence(low)"  ),
                         tr( "95%_Confidence(high)" ) );
      // Show summary of reaction attributes;
      for ( int ii = 0; ii < nreac; ii++ )
      {
         QString reacnum = QString().sprintf( "%2d", ii + 1 );
         bool is_fixed   = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );
         QString strclo  = is_fixed ? fixd :
                            QString().sprintf( "%10.4e", mstats[ kd ][  9 ] );
         QString strchi  = is_fixed ? blnk :
                            QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] );
         mstr += table_row( reacnum, tr( "K_dissociation" ),
                            QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ),
                            strclo, strchi );
         kd++;
         is_fixed        = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );
         strclo          = is_fixed ? fixd :
                            QString().sprintf( "%10.4e", mstats[ kd ][  9 ] );
         strchi          = is_fixed ? blnk :
                            QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] );
         mstr += table_row( reacnum, tr( "K_off Rate" ),
                            QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ),
                            strclo, strchi );
         kd++;
      }

      mstr += indent( 2 ) + "</table>\n";

      // Compose the details statistics entries
      mstr += indent( 2 );
      mstr += tr( "<h3>Discrete Model GA-MC Detailed Statistics:</h3>\n" );
      int icomp       = 1;
      int ireac       = 1;
      int kdmax       = 6;
      int kk          = 0;

      // First, the RMSDs
      mstr += indent( 2 ) + tr( "<h4>Details for MC Iteration RMSDs:</h4>\n" );
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Minimum:" ),
              QString().sprintf( "%10.4e", rstats[  0 ] ) );
      mstr += table_row( tr( "Maximum:" ),
              QString().sprintf( "%10.4e", rstats[  1 ] ) );
      mstr += table_row( tr( "Mean:" ),
              QString().sprintf( "%10.4e", rstats[  2 ] ) );
      mstr += table_row( tr( "Median:" ),
              QString().sprintf( "%10.4e", rstats[  3 ] ) );
      mstr += table_row( tr( "Skew:" ),
              QString().sprintf( "%10.4e", rstats[  4 ] ) );
      mstr += table_row( tr( "Kurtosis:" ),
              QString().sprintf( "%10.4e", rstats[  5 ] ) );
      mstr += table_row( tr( "Lower Mode:" ),
              QString().sprintf( "%10.4e", rstats[  6 ] ) );
      mstr += table_row( tr( "Upper Mode:" ),
              QString().sprintf( "%10.4e", rstats[  7 ] ) );
      mstr += table_row( tr( "Mode Center:" ),
              QString().sprintf( "%10.4e", rstats[  8 ] ) );
      mstr += table_row( tr( "95% Confidence Interval Low:" ),
              QString().sprintf( "%10.4e", rstats[  9 ] ) );
      mstr += table_row( tr( "95% Confidence Interval High:" ),
              QString().sprintf( "%10.4e", rstats[ 10 ] ) );
      mstr += table_row( tr( "99% Confidence Interval Low:" ),
              QString().sprintf( "%10.4e", rstats[ 11 ] ) );
      mstr += table_row( tr( "99% Confidence Interval High:" ),
              QString().sprintf( "%10.4e", rstats[ 12 ] ) );
      mstr += table_row( tr( "Standard Deviation:" ),
              QString().sprintf( "%10.4e", rstats[ 13 ] ) );
      mstr += table_row( tr( "Standard Error:" ),
              QString().sprintf( "%10.4e", rstats[ 14 ] ) );
      mstr += table_row( tr( "Variance:" ),
              QString().sprintf( "%10.4e", rstats[ 15 ] ) );
      mstr += table_row( tr( "Correlation Coefficient:" ),
              QString().sprintf( "%10.4e", rstats[ 16 ] ) );
      mstr += table_row( tr( "Number of Bins:" ),
              QString().sprintf( "%10.0f", rstats[ 17 ] ) );
      mstr += table_row( tr( "Distribution Area:" ),
              QString().sprintf( "%10.4e", rstats[ 18 ] ) );
      mstr += table_row( tr( "95% Confidence Limit Low:" ),
              QString().sprintf( "%10.4e", rstats[ 19 ] ) );
      mstr += table_row( tr( "95% Confidence Limit High:" ),
              QString().sprintf( "%10.4e", rstats[ 20 ] ) );
      mstr += table_row( tr( "99% Confidence Limit Low:" ),
              QString().sprintf( "%10.4e", rstats[ 21 ] ) );
      mstr += table_row( tr( "99% Confidence Limit High:" ),
              QString().sprintf( "%10.4e", rstats[ 22 ] ) );
      mstr += indent( 2 ) + "</table>\n";

      // Then, components and attributes
      for ( kd = 0; kd < ntatt; kd++ )
      {
         QString compnum = tr( "Component %1 " ).arg( icomp );
         QString reacnum = tr( "Reaction %1 "  ).arg( ireac );
         QString attrib  = compnum + atitl[ kk ];
         mstr += indent( 2 ) + "<h4>" + tr( "Details for " );

         if ( icomp <= ncomp )
         {  // Title for component detail
            if ( imodels[ 0 ].is_product( icomp - 1 )  &&  kk == 0 )
               mstr += compnum + tr( "(Product) Total Concentration" )
                     + ":</h4>\n";
            else
               mstr += compnum + atitl[ kk ] + ":</h4>\n";
         }
         else
         {  // Title for reaction detail
            mstr += reacnum + rtitl[ kk ] + ":</h4>\n";
         }

         mstr += indent( 2 ) + "<table>\n";
         bool is_fixed   = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );

         if ( is_fixed )
         {  // Fixed has limited lines
            mstr += table_row( tr( "Minimum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  0 ] ) );
            mstr += table_row( tr( "Maximum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  1 ] ) );
            mstr += table_row( tr( "Mean:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ) );
            mstr += table_row( tr( "Median (Fixed)" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  3 ] ) );
         }

         else
         {  // Float has full set of statistics details
            mstr += table_row( tr( "Minimum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  0 ] ) );
            mstr += table_row( tr( "Maximum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  1 ] ) );
            mstr += table_row( tr( "Mean:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ) );
            mstr += table_row( tr( "Median:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  3 ] ) );
            mstr += table_row( tr( "Skew:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  4 ] ) );
            mstr += table_row( tr( "Kurtosis:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  5 ] ) );
            mstr += table_row( tr( "Lower Mode:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  6 ] ) );
            mstr += table_row( tr( "Upper Mode:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  7 ] ) );
            mstr += table_row( tr( "Mode Center:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  8 ] ) );
            mstr += table_row( tr( "95% Confidence Interval Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  9 ] ) );
            mstr += table_row( tr( "95% Confidence Interval High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] ) );
            mstr += table_row( tr( "99% Confidence Interval Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 11 ] ) );
            mstr += table_row( tr( "99% Confidence Interval High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 12 ] ) );
            mstr += table_row( tr( "Standard Deviation:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 13 ] ) );
            mstr += table_row( tr( "Standard Error:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 14 ] ) );
            mstr += table_row( tr( "Variance:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 15 ] ) );
            mstr += table_row( tr( "Correlation Coefficient:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 16 ] ) );
            mstr += table_row( tr( "Number of Bins:" ),
                    QString().sprintf( "%10.0f", mstats[ kd ][ 17 ] ) );
            mstr += table_row( tr( "Distribution Area:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 18 ] ) );
            mstr += table_row( tr( "95% Confidence Limit Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 19 ] ) );
            mstr += table_row( tr( "95% Confidence Limit High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 20 ] ) );
            mstr += table_row( tr( "99% Confidence Limit Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 21 ] ) );
            mstr += table_row( tr( "99% Confidence Limit High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 22 ] ) );
         }

         mstr += indent( 2 ) + "</table>\n";

         if ( (++kk) >= kdmax )
         {
            kk              = 0;
            icomp++;
            if ( icomp > ncomp )
            {
               ireac           = icomp - ncomp;
               kdmax           = 2;
            }
         }
      }
   }
   */

   return mstr;
}


// String to accomplish line indentation
QString  US_ReporterGMP::indent( const int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

// Table row HTML with 2 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2 ) const
{
   return( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

// Table row HTML with 3 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
            + "</td></tr>\n" );
}

// Table row HTML with 5 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
            + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n" );
}

// Table row HTML with 7 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5, const QString& s6,
                               const QString& s7 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>" + s7 + "</td></tr>\n" );
}


// Open a residual plot dialog
void US_ReporterGMP::plotres( )
{
  QStringList PlotsFileNames;
  mkdir( US_Settings::reportDir(), edata->runID );
  const QString svgext( ".svgz" );
  const QString pngext( ".png" );
  const QString csvext( ".csv" );
  QString tripnode  = QString( currentTripleName ).replace( ".", "" );
  QString basename  = US_Settings::reportDir() + "/" + edata->runID + "/" + text_model( model, 0 ) + "." + tripnode + ".";

  QString img01File = basename + "velocity_nc" + svgext;
  QString img02File = basename + "residuals"   + pngext;
  QString img03File = basename + "s_distrib"   + svgext;
  QString img04File = basename + "mw_distrib"  + svgext;
  QString img05File = basename + "D_distrib"   + svgext;
  QString img06File = basename + "ff0_vs_s"    + svgext;
  QString img07File = basename + "ff0_vs_mw"   + svgext;
  QString img08File = basename + "D_vs_s"      + svgext;
  QString img09File = basename + "D_vs_mw"     + svgext;
  QString img10File = basename + "3dplot"      + pngext;

  // Plots for Exp-Sim ovelray (with noises rmoved && residual plot)
  resplotd = new US_ResidPlotFem( this, "REPORT" );
  
  write_plot( img01File, resplotd->rp_data_plot1() );  //<-- rp_data_plot1() gives overlay (Exp/Simulations) plot
  PlotsFileNames <<  basename + "velocity_nc"   + pngext;
  
  write_plot( img02File, resplotd->rp_data_plot2() );  //<-- rp_data_plot2() gives residuals plot
  PlotsFileNames << img02File;

  //Stick Plots for S-, MW-, D- distributions
  plotLayout1 = new US_Plot( data_plot1,
			     tr( "Residuals" ),
			     tr( "Radius (cm)" ),
			     tr( "OD Difference" ),
			     true, "^resids [0-9].*", "rainbow" );
  
  plotLayout2 = new US_Plot( data_plot2,
			     tr( "Velocity Data" ),
			     tr( "Radius (cm)" ),
			     tr( "Absorbance" ),
			     true, ".*in range", "rainbow" );
  
  data_plot1->setMinimumSize( 560, 240 );
  data_plot2->setMinimumSize( 560, 240 );
    
  distrib_plot_stick( 0 );               // s-distr.
  write_plot( img03File, data_plot1 );
  PlotsFileNames <<  basename + "s_distrib"   + pngext;
  
  distrib_plot_stick( 1 );
  write_plot( img04File, data_plot1 );   // MW-distr.
  PlotsFileNames <<  basename + "mw_distrib"  + pngext;
  
  distrib_plot_stick( 2 );
  write_plot( img05File, data_plot1 );   // D-distr.
  PlotsFileNames <<  basename + "D_distrib"  + pngext;

  //2D distributions: ff0_vs_s, ff0_vs_mw, D_vs_s, D_vs_mw
  distrib_plot_2d( ( cnstvb ? 3 : 5 ) );
  write_plot( img06File, data_plot1 );
  PlotsFileNames <<  basename + "ff0_vs_s"  + pngext;
  
  distrib_plot_2d( ( cnstvb ? 4 : 6 ) );
  write_plot( img07File, data_plot1 );
  PlotsFileNames <<  basename + "ff0_vs_mw"  + pngext;

  distrib_plot_2d(    7 );
  write_plot( img08File, data_plot1 );
  PlotsFileNames <<  basename + "D_vs_s"  + pngext;

  distrib_plot_2d(    8 );
  write_plot( img09File, data_plot1 );
  PlotsFileNames <<  basename + "D_vs_mw"  + pngext;

  //3D plot
  write_plot( img10File, NULL );
  PlotsFileNames <<  img10File;
  
  //add .PNG plots to combined PDF report
  assemble_plots_html( PlotsFileNames  );
}


// Do stick type distribution plot
void US_ReporterGMP::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( " :\nCell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Rel. Concentr." );

   if ( type == 0 )
   {
      pltitle = pltitle + tr( "\ns20,W Distribution" );
      xatitle = tr( "Corrected Sedimentation Coefficient" );
   }

   else if ( type == 1 )
   {
      pltitle = pltitle + tr( "\nMW Distribution" );
      xatitle = tr( "Molecular Weight (Dalton)" );
   }

   else if ( type == 2 )
   {
      pltitle = pltitle + tr( "\nD20,W Distribution" );
      xatitle = tr( "D20,W (cm^2/sec)" );
   }

   dataPlotClear( data_plot1 );

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*  data_grid = us_grid(  data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );


   int     dsize  = model_used.components.size();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( type == 0 ) ? model_used.components[ jj ].s :
               ( ( type == 1 ) ? model_used.components[ jj ].mw :
                                 model_used.components[ jj ].D );
      yval     = model_used.components[ jj ].signal_concentration;
      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = ( type == 0 ) ? xmin : max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin ( true );
   data_grid->enableY    ( true );
   data_grid->setMajorPen(
      QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );

   data_curv->setSamples( xx, yy, dsize );
   data_curv->setPen    ( QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle  ( QwtPlotCurve::Sticks );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();
}

// Do 2d type distribution plot
void US_ReporterGMP::distrib_plot_2d( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( " :\nCell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString yatitle;
   QString xatitle;

   if ( type == 3 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Sed. Coeff." );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 4 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Mol. Weight" );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 5 )
   {
      pltitle = pltitle + tr( "\nVbar vs Sed. Coeff." );
      yatitle = tr( "Vbar at 20" ) + DEGC;
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 6 )
   {
      pltitle = pltitle + tr( "\nVbar vs Mol. Weight" );
      yatitle = tr( "Vbar at 20" ) + DEGC;
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 7 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Sed. Coeff." );
      yatitle = tr( "Diff. Coeff. D20,W" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 8 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Molecular Weight" );
      yatitle = tr( "Diff. Coeff. D20,W" );
      xatitle = tr( "Molecular Weight" );
   }

   dataPlotClear( data_plot1 );

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*  data_grid = us_grid(  data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );
   QwtSymbol*    symbol    = new QwtSymbol;

   int     dsize  = model_used.components.size();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( ( type & 1 ) == 1 ) ? model_used.components[ jj ].s :
                                         model_used.components[ jj ].mw;

      if ( type < 5 )             yval = model_used.components[ jj ].f_f0;
      else if ( type < 7 )        yval = model_used.components[ jj ].vbar20;
      else                        yval = model_used.components[ jj ].D;

      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = ( type & 1 ) == 1 ? xmin : max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin ( true );
   data_grid->enableY    ( true );
   data_grid->setMajorPen(
      QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );

   symbol->setStyle( QwtSymbol::Ellipse );
   symbol->setPen(   QPen(   Qt::red    ) );
   symbol->setBrush( QBrush( Qt::yellow ) );
   if ( dsize > 100 )
      symbol->setSize(  5 );
   else if ( dsize > 50 )
      symbol->setSize(  8 );
   else if ( dsize > 20 )
      symbol->setSize( 10 );
   else
      symbol->setSize( 12 );

   data_curv->setStyle  ( QwtPlotCurve::NoCurve );
   data_curv->setSymbol ( symbol );
   data_curv->setSamples( xx, yy, dsize );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();
}

// Create a subdirectory if need be
bool US_ReporterGMP::mkdir( const QString& baseDir, const QString& subdir )
{
   QDir folder( baseDir );

   if ( folder.exists( subdir ) ) return true;

   if ( folder.mkdir( subdir ) ) return true;

   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );

   return false;
}

// Write out a plot
void US_ReporterGMP::write_plot( const QString& filename, const QwtPlot* plot )
{
   if ( filename.contains( ".svg" ) )
   {  // Save an SVG file and a PNG copy
      if ( US_GuiUtil::save_plot( filename, plot ) != 0 )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }
   
//    else if ( filename.endsWith( "rbitmap.png" ) )
//    {  // Special case of rbitmap PNG
//       if ( rbmapd == 0 )
//       {  // if it is not currently displayed, display it
//          rbmapd = new US_ResidsBitmap( resids );
//          rbmapd->move( bmd_pos );
//          rbmapd->show();
//          rbmapd->raise();
//       }

//       else
//       {  // if already displayed,  replot and re-activate
//          rbmapd->replot( resids );
//          rbmapd->raise();
//          rbmapd->activateWindow();
//       }

// #if QT_VERSION > 0x050000
//       QPixmap pixmap = ((QWidget*)rbmapd)->grab();
// #else
//       QPixmap pixmap = QPixmap::grabWidget( rbmapd, 0, 0,
//                                             rbmapd->width(), rbmapd->height() );
// #endif

//       if ( ! pixmap.save( filename ) )
//          QMessageBox::warning( this, tr( "File Write Error" ),
//             tr( "Unable to write file" ) + filename );
//    }

    else if ( filename.endsWith( "3dplot.png" ) )
    {  // Special case of 3dplot PNG
       // if ( eplotcd == 0 )
       // {  // if no 3d plot control up,  create it now
          eplotcd = new US_PlotControlFem( this, &model );
          // eplotcd->move( epd_pos );
          // eplotcd->show();
          eplotcd->do_3dplot_auto();
	  //}

 #if defined(Q_OS_WIN) || defined(Q_OS_MAC)
       qDebug() << "3D: Q_OS_WIN || Q_OS_MAC ";	  
       US_Plot3D* widgw = eplotcd->widget_3dplot();
       bool ok          = widgw->save_plot( filename, QString( "png" ) );
 #else
       qDebug() << "3D: Q_OS_LINUX || other";	  
       QGLWidget* dataw = eplotcd->data_3dplot();
       QPixmap pixmap   = dataw->renderPixmap( dataw->width(), dataw->height(),
                                             true  );
       bool ok          = pixmap.save( filename );
 #endif

       if ( ! ok )
          QMessageBox::warning( this, tr( "File Write Error" ),
             tr( "Unable to write file" ) + filename );
    }

   else if ( filename.endsWith( ".png" ) )
   {  // General case of PNG
      if ( US_GuiUtil::save_png( filename, plot ) != 0 )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }
}


// Public slot to mark residuals plot dialog closed
void US_ReporterGMP::resplot_done()
{
  qDebug() << "RESPLOT being closed -- ";

  resplotd   = 0; // <--- TEMP
}


// Update progress when thread reports
void US_ReporterGMP::thread_progress( int thr, int icomp )
{
  qDebug() <<  "Updating progress multiple threads, thr, icomp -- " << thr << icomp;
   int kcomp     = 0;
   kcomps[ thr ] = icomp;
   for ( int ii = 0; ii < nthread; ii++ )
      kcomp += kcomps[ ii ];
   progress_msg->setValue( kcomp );
   qApp->processEvents();
   qDebug() << "THR PROGR thr icomp" << thr << icomp << "kcomp" << kcomp;
}


// Update count of threads completed and colate simulations when all are done
void US_ReporterGMP::thread_complete( int thr )
{
   thrdone++;
   qDebug() << "THR COMPL thr" << thr << "thrdone" << thrdone;

   if ( thrdone >= nthread )
   {  // All threads are done, so sum thread simulation data
      for ( int ii = 0; ii < sdata->scanData.size(); ii++ )
      {
         for ( int jj = 0; jj < sdata->xvalues.size(); jj++ )
         {
            //double conc = 0.0;
            double conc = sdata->value( ii, jj );

            for ( int kk = 0; kk < nthread; kk++ )
               conc += tsimdats[ kk ].value( ii, jj );

            sdata->setValue( ii, jj, conc );
         }
      }

      // Then show the results
      show_results();
   }
}

// Adjust model components based on buffer, vbar, and temperature
void US_ReporterGMP::adjustModel( )
{
   model              = model_used;

   // build model component correction factors
   double avgTemp     = edata->average_temperature();
   double vbar20      = svbar_global.toDouble();

   solution.density   = density;
   solution.viscosity = viscosity;
   solution.manual    = manual;
   solution.vbar20    = vbar20;
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
//   solution.vbar      = US_Math2::adjust_vbar20( solution.vbar20, avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   double scorrec  = solution.s20w_correction;
   double dcorrec  = solution.D20w_correction;
   double mc_vbar  = model.components[ 0 ].vbar20;
   // Set constant-vbar and constant-ff0 flags
   cnstvb          = model.constant_vbar();
   cnstff          = model.constant_ff0();

   US_Math2::SolutionData sd;
   sd.density      = solution.density;
   sd.viscosity    = solution.viscosity;
   sd.vbar20       = solution.vbar20;
   sd.vbar         = solution.vbar;
   sd.manual       = solution.manual;
   qDebug() << "Fem:Adj:  avgT" << avgTemp << "scorr dcorr" << scorrec << dcorrec;

   if ( cnstvb  &&  mc_vbar != sd.vbar20  &&  mc_vbar != 0.0 )
   {  // Use vbar from the model component, instead of from the solution
      sd.vbar20    = mc_vbar;
      sd.vbar      = US_Math2::adjust_vbar20( sd.vbar20, avgTemp );
      US_Math2::data_correction( avgTemp, sd );
      scorrec      = sd.s20w_correction;
      dcorrec      = sd.D20w_correction;
      qDebug() << "Fem:Adj:   cnstvb" << cnstvb << "  scorr dcorr" << scorrec << dcorrec;
   }
   qDebug() << "Fem:Adj:  avgT" << avgTemp << "vb20 vb" << sd.vbar20 << sd.vbar;

   // Convert to experiment space: adjust s,D based on solution,temperature

   for ( int jj = 0; jj < model.components.size(); jj++ )
   {
      US_Model::SimulationComponent* sc = &model.components[ jj ];

      sc->vbar20  = ( sc->vbar20 == 0.0 ) ? vbar20 : sc->vbar20;

      if ( ! cnstvb )
      {  // Set s,D corrections based on component vbar
         sd.vbar20   = sc->vbar20;
         sd.vbar     = US_Math2::adjust_vbar20( sd.vbar20, avgTemp );
         US_Math2::data_correction( avgTemp, sd );
         scorrec     = sd.s20w_correction;
         dcorrec     = sd.D20w_correction;
      }

double s20w=sc->s;
double d20w=sc->D;
      sc->s      /= scorrec;
      sc->D      /= dcorrec;
      qDebug() << "Fem:Adj:  s20w D20w" << s20w << d20w
	       << "s D" << sc->s << sc->D << "  jj" << jj << "vb20 vb" << sc->vbar20 << sd.vbar;

      if ( sc->extinction > 0.0 )
         sc->molar_concentration = sc->signal_concentration / sc->extinction;
   }

}


//Parse filename and extract one for given optics type in combined runs
QString US_ReporterGMP::get_filename( QString triple_name )
{
  qDebug() << "FileName -- " << FileName;
  qDebug() << "triple_name -- " << triple_name;
 
  QString filename_parsed;
  
  if ( FileName.contains(",") && FileName.contains("IP") && FileName.contains("RI") )
    {
      QStringList fileList  = FileName.split(",");

      //Interference
      if ( triple_name.contains("Interference") )
	{
	  for (int i=0; i<fileList.size(); ++i )
	    {
	      QString fname = fileList[i];
	      int pos = fname.lastIndexOf(QChar('-'));
	      qDebug() << "IP: pos -- " << pos;
	      qDebug() << "IP: fname.mid( pos ) -- " << fname.mid( pos );
	      if ( fname.mid( pos ) == "-IP")
		{
		  filename_parsed = fname;
		  break;
		}
	    }
	}
      //UV/vis.
      else
	{
	  for (int i=0; i<fileList.size(); ++i )
	    {
	      QString fname = fileList[i];
	      int pos = fname.lastIndexOf(QChar('-'));
	      qDebug() << "RI: pos -- " << pos;
	      qDebug() << "RI: fname.mid( pos ) -- " << fname.mid( pos );
	      if ( fname.mid( pos ) == "-RI")
		{
		  filename_parsed = fname;
		  break;
		}
	    }
	}
    }
  else
    filename_parsed = FileName;

  qDebug() << "Parsed filename: " <<  filename_parsed;

  return filename_parsed;
}

//Start assembling PDF file
void US_ReporterGMP::assemble_pdf()
{

  QString rptpage;

  // Compose the report header
  rptpage   = QString( "<?xml version=\"1.0\"?>\n" );
  rptpage  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
  rptpage  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
              "/xhtml1-strict.dtd\">\n";
  rptpage  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
              " xml:lang=\"en\" lang=\"en\">\n";
  rptpage  += "  <head>\n";
  rptpage  += "  <title> Ultrascan III Composite Report </title>\n";
  rptpage  += "  <meta http-equiv=\"Content-Type\" content="
              "\"text/html; charset=iso-8859-1\"/>\n";
  rptpage  += "  <style type=\"text/css\" >\n";
  rptpage  += "    td { padding-right: 0.75em; }\n";
  rptpage  += "    body { background-color: white; }\n";
  rptpage  += "    .pagebreak\n";
  rptpage  += "    {\n";
  rptpage  += "      page-break-before: always; border: 1px solid; \n";
  rptpage  += "    }\n";
  rptpage  += "    .parahead\n";
  rptpage  += "    {\n";
  rptpage  += "      font-weight: bold;\n";
  rptpage  += "      font-style:  italic;\n";
  rptpage  += "    }\n";
  rptpage  += "    .datatext\n";
  rptpage  += "    {\n";
  rptpage  += "      font-family: monospace;\n";
  rptpage  += "    }\n";
  rptpage  += "  </style>\n";
  rptpage  += "  </head>\n  <body>\n";
   
  
  //HEADER: begin
  QString html_header = QString("");
  html_header += rptpage;
  html_header += tr( 
    "<div align=left>"
      "Created, %1<br>"
      "with UltraScan-GMP<br>"
      "by AUC Solutions<br>"
    "</div>"
		     )
    .arg( current_date )
    ;
  //HEADER: end

  
  //TITLE: begin
  QString html_title = tr(
    "<h1 align=center>GMP REPORT FOR RUN <br><i>%1</i></h1>"
    "<hr>"
			  )
    .arg( currProto. protoname + "-run" + runID )    //1
    ;
  //TITLE: end

  QString html_paragraph_open = tr(
    "<p align=justify>"
				   )
    ;
  
  //GENERAL: begin
  html_general = tr(
    
    "<h3 align=left>General Settings</h3>"
      "<table>"
        "<tr><td>Investigator: </td>                           <td>%1</td></tr>"
        "<tr><td>Run Name:</td>                                <td>%2</td></tr>"
        "<tr><td>Project:</td>                                 <td>%3</td></tr>"
        "<tr><td>Run Temperature (&#8451;):</td>               <td>%4</td></tr>"
        "<tr><td>Temperature-Equilibration Delay (mins):</td>  <td>%5</td></tr>"
      "</table>"
    "<hr>"
			    )
    .arg( currProto. investigator)  //1
    //.arg( currProto. runname)       //2  
    .arg( currProto. protoname + "-run" + runID) //2
    .arg( currProto. project)       //3
    .arg( currProto. temperature)   //4
    .arg( currProto. temeq_delay)   //5
    ;
  //GENERAL: end


  //ROTOR/LAB: begin
  html_lab_rotor = tr(
    "<h3 align=left>Lab/Rotor Parameters</h3>"
      "<table>"
        "<tr><td>Laboratory:</td>      <td>%1</td></tr>"
        "<tr><td>Rotor: </td>          <td>%2</td></tr>"
        "<tr><td>Calibration Date:</td><td>%3</td></tr>"
      "</table>"
    "<hr>"
			      )
    .arg( currProto. rpRotor.laboratory )  //1
    .arg( currProto. rpRotor.rotor )       //2
    .arg( currProto. rpRotor.calibration)  //3
    ;
  //ROTOR/LAB: end 	      
  
  //OPERATOR: begin
  html_operator = tr(     
    "<h3 align=left>Optima Machine/Operator </h3>"
      "<table>"
        "<tr><td>Optima: </td>           <td>%1</td></tr>"
        "<tr><td>Operator: </td>         <td>%2</td></tr>"
        "<tr><td>Experiment Type:</td>   <td>%3</td></tr>"
      "</table>"
    "<hr>"
				  )
    .arg( currProto. rpRotor.instrname )   //1
    .arg( currProto. rpRotor.opername  )   //2
    .arg( currProto. rpRotor.exptype )     //3
    ;
  //OPERATOR: end 	  

  
  //SPEEDS: begin
  html_speed = tr(
    "<h3 align=left>Speed Parameters </h3>"
      "<table>"
        "<tr><td>Rotor Speed  (RPM):    </td>                   <td>%1</td></tr>"
        "<tr><td>Acceleration (RMP/sec): </td>                  <td>%2</td></tr>"
        "<tr><td>Active Scaning Time:    </td>                  <td>%3</td></tr>"
        "<tr><td>Stage Delay:          </td>                    <td>%4</td></tr>"
        "<tr><td>Total Time (without equilibration):  </td>     <td>%5</td></tr>"
        "<tr><td><br><b><i>UV-visible optics (total):</i></b>   </td>  "
        "<tr><td>Delay to First Scan:            </td>          <td>%6</td></tr>"
        "<tr><td>Scan Interval:                  </td>          <td>%7</td></tr>"
        "<tr><td><br><b><i>Interference optics (per cell):</i></b></td>   "
        "<tr><td>Delay to First Scan:  </td>                    <td>%8</td></tr>"
        "<tr><td>Scan Interval:        </td>                    <td>%9</td></tr>"
      "</table>"
    "<hr>"
			  )
    .arg( QString::number ( currProto. rpSpeed. ssteps[0].speed ) )       //1
    .arg( QString::number ( currProto. rpSpeed. ssteps[0].accel ) )       //2
    .arg( duration_str    )                                               //3
    .arg( delay_stage_str )                                               //4
    .arg( total_time_str  )                                               //5
    .arg( delay_uvvis_str )                                               //6
    .arg( scanint_uvvis_str )                                             //7
    .arg( delay_int_str   )                                               //8
    .arg( scanint_int_str )                                               //9
    ;
  //SPEEDS: end


  //CELLS: begin
  html_cells = tr(
    "<h3 align=left>Cell Centerpiece Usage </h3>"
			   )
     ;
   
  ncells_used = currProto. rpCells. nused;
  html_cells += tr(
     "# of Used Cells:  %1<br>"
		    )
    .arg( QString::number ( ncells_used ) )                                        //1
    ;
   
  html_cells += tr(
      "<table>"
		   )
    ;

  for ( int i=0; i<ncells_used; ++i )
    {
      QString cell        = QString::number( currProto. rpCells. used[i].cell );
      QString centerpiece = currProto. rpCells. used[i].centerpiece;
      QString cell_label  = QString( tr( "Centerpiece" ) );
      QString window      = currProto. rpCells. used[i].windows;
      
      //check if last cell counterbalance
      if ( i == ncells_used - 1 && 
	   currProto. rpCells. used[i].cbalance.contains( tr("counterbalance") ) )
	{
	  cell_label = tr("Counterbalance");
	  centerpiece = currProto. rpCells. used[i].cbalance;
	  window = tr("N/A");
	}
	
      html_cells += tr(
		       "<tr>" 
		       "<td>Cell Number:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp;" 
		       "<td> %2: </td><td> %3 </td> &nbsp;&nbsp;&nbsp;&nbsp;"
		       "<td>Windows:</td><td> %4 </td>"
		       "</tr>"
			)
	.arg( cell )                          //1
	.arg( cell_label )                    //2
	.arg( centerpiece )                   //3
	.arg( window )                        //4 
	;
    }
  
  html_cells += tr(
      "</table>"
    "<hr>"
		   )
    ;
  //CELLS: end

  
  //SOLUTIONS: begin
  html_solutions = tr(
		      "<h3 align=left>Solutions for Channels</h3>"			      
		      )
    ;
   
  nsol_channels = currProto.rpSolut.nschan;
  html_solutions += tr(
     "# of Solution Channels:  %1 <br>"
		    )
    .arg( nsol_channels )                                                   //1
    ;
  
  for ( int i=0; i<nsol_channels; ++i )
    {
      html_solutions += tr(
			   "<table>"		   
			   "<tr>"
			      "<td><b>Cell/Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			   "</tr>"
			   "</table>"
			)
	.arg( currProto. rpSolut. chsols[i].channel )                       //1
	;

      QString sol_id      = currProto. rpSolut. chsols[i].sol_id;
      QString sol_comment = currProto. rpSolut. chsols[i].ch_comment;
      add_solution_details( sol_id, sol_comment, html_solutions );

      progress_msg->setValue( progress_msg->value() + i );
      qApp->processEvents();
    }

  html_solutions += tr( "<hr>" );
  //SOLUTIONS: end


  //OPTICS: begin
  html_optical = tr(
		    "<h3 align=left>Optical Systems </h3>"
		    )
    ;
  nchan_optics = currProto. rpOptic. nochan;
  html_optical += tr(
		     "# of Channels With Optics:  %1<br>"
		     )
    .arg( QString::number( nchan_optics ))                                                     //1
    ;
  html_optical += tr(
		     "<table>"
		     );

  for ( int i=0; i<nchan_optics; ++i )
    {
      QString channel  = currProto. rpOptic. chopts[i].channel;
      QString scan1    = currProto. rpOptic. chopts[i].scan1;
      QString scan2    = currProto. rpOptic. chopts[i].scan2;
      QString scan3    = currProto. rpOptic. chopts[i].scan3;

      // //test
      // if ( i == 3 )
      // 	scan2 = tr( "Interf." );
      // if ( i == nchan_optics - 2 )
      // 	scan3 = tr( "Fluorescense" );
      // if ( i == nchan_optics - 1 )
      // 	scan2 = tr( "Interference" );
      // //////

      
      html_optical += tr(
			 "<tr>" 
			    "<td>Cell/Channel:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			 )
	.arg( channel );                      //1
      
      
      if ( !scan1.isEmpty() )
	{
	  has_uvvis = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan1 );                    //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>");
      
      if ( !scan2.isEmpty() )
	{
	  has_interference = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan2 );                        //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>" );    
      
      if ( !scan3.isEmpty() )
	{
	  has_fluorescense = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan3 );                        //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>" );    
           
      
      html_optical += tr(
			 "</tr>"
			 );
    }
  
  html_optical += tr(
      "</table>"
    "<hr>"
		   )
    ;
  //OPTICS: end


  //RANGES: begin
  html_ranges = tr(
		   "<h3 align=left> Ranges </h3>"
		   )
    ;
  nchan_ranges  = currProto. rpRange. nranges;
  html_ranges  += tr(
		     "# of Channels With Ranges:  %1<br>"
		     )
    .arg( QString::number( nchan_ranges ))                                  //1
    ;
  
  for ( int i=0; i < nchan_ranges; ++i )
    {
      html_ranges += tr(
			"<table>"		   
			  "<tr>"
			    "<td><b>Cell/Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			  "</tr>"
			"</table>"
			)
	.arg( currProto. rpRange. chrngs[i].channel )                       //1
	;

      int w_count     = currProto. rpRange. chrngs[i].wvlens.size();
      double  w_min   = currProto. rpRange. chrngs[i].wvlens[0];
      double  w_max   = currProto. rpRange. chrngs[i].wvlens[ w_count - 1 ];
      double  r_min   = currProto. rpRange. chrngs[i].lo_rad;
      double  r_max   = currProto. rpRange. chrngs[i].hi_rad;
      QString w_range = QString::number( w_min ) + tr(" to ") + QString::number( w_max );
      QString r_range = QString::number( r_min ) + tr(" to ") + QString::number( r_max );

      //wavelengths:
      QString all_wvl = QString("");
      QList< double > wvl_list = currProto. rpRange. chrngs[i].wvlens;
      int counter = 0;
      for (int  jj =0; jj < w_count; jj++)
	{
	  ++counter;
	  all_wvl += tr("%1").arg( wvl_list[jj] );
         if( jj != wvl_list.size() -1 )
	   all_wvl += tr(", ");
         if(counter % 8 == 0)
	   all_wvl += tr("<br>");
	}
      
      html_ranges += tr(
			"<table style=\"margin-left:30px\">"
			  "<tr><td> Selected Wavelength count: </td>  <td> %1 </td> </tr>"
			  "<tr><td> Selected Wavelength range: </td>  <td> %2 </td> </tr>"
			  "<tr><td> Radius range:              </td>  <td> %3 </td> </tr>"
			  "<tr><td> Selected Wavelengths:      </td>  <td> %4 </td> </tr>"
			"</table>"
			)
	.arg( QString::number( w_count ) )        //1
	.arg( w_range )                           //2
	.arg( r_range )                           //3
	.arg( all_wvl )                           //4	
	;
      
    }  

  html_ranges += tr( "<hr>" ) ;
  //RANGES: end

  
  //SCAN_COUNT: begin
  html_scan_count = tr(
		       "<h3 align=left> Scan Counts and Scan Intervals For Optics in Use </h3>"
		       "&nbsp;&nbsp;<br>"
		       )
    ;

  double scanintv     = currProto. rpSpeed. ssteps[0].scanintv;
  double scanintv_int = currProto. rpSpeed. ssteps[0].scanintv_int;
  int scancount       = currProto. rpSpeed. ssteps[0].scancount;
  int scancount_int   = currProto. rpSpeed. ssteps[0].scancount_int;

  //UV-vis
  QString html_scan_count_uv = tr(
				  "<table>"		   
				    "<tr>"
				      "<td><b><i>UV-visible optics:</i></b></td>"
				    "</tr>"
				  "</table>"
			)
    ;
  html_scan_count_uv += tr(
			   "<table style=\"margin-left:30px\">"
			     "<tr><td> Scan Interval:             </td>  <td> %1 </td> </tr>"
			     "<tr><td> # Scans per Triple:        </td>  <td> %2 </td> </tr>"
			   "</table>"
			   )
    .arg( scanint_uvvis_str )                             //1
    .arg( QString::number( scancount ) )                  //2
    ;
  
  //Interference
  QString html_scan_count_int = tr(
				   "<table>"		   
				     "<tr>"
				       "<td><b><i>Interference optics:</i></b></td>"
				     "</tr>"
				   "</table>"
				   )
    ;
  html_scan_count_int += tr(
			    "<table style=\"margin-left:30px\">"
			      "<tr><td> Scan Interval:             </td>  <td> %1 </td> </tr>"
			      "<tr><td> # Scans per Cell:          </td>  <td> %2 </td> </tr>"
			    "</table>"
			   )
    .arg( scanint_int_str )                                //1
    .arg( QString::number( scancount_int ) )               //2
    ;


  if ( has_uvvis )
    html_scan_count += html_scan_count_uv;
  if ( has_interference )
    html_scan_count += html_scan_count_int;  
  
  html_scan_count += tr( "<hr>" ) ;
  //SCAN_COUNT: end

  
  //APROFILE: begin
  //ANALYSIS: General settings && Reports: begin
  html_analysis_profile = tr(
			     "<h3 align=left> Analysis Profile: General Settings and Reports  </h3>"
			     "&nbsp;&nbsp;<br>"
			     )
    ;

  //Begin of the General Analysis Section
  QString html_analysis_gen;
  
  html_analysis_gen += tr(
			  "<table>"		   
			  "<tr> <td> Profile Name:  &nbsp;&nbsp;&nbsp;&nbsp; </td>  <td> %1 </td></tr>"
			  "<tr> <td> Protocol Name: &nbsp;&nbsp;&nbsp;&nbsp; </td>  <td> %2 </td></tr>"
			  "</table>"
			  "<br>"
			  )
    .arg( currAProf.aprofname  )         //1
    .arg( currAProf.protoname  )         //2
    ;

  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = chndescs_alt[ i ];
      QString channel_desc     = chndescs[ i ];
      
      html_analysis_gen += tr(
			      "<table>"		   
			      "<tr>"
			      "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			      "</tr>"
			      "</table>"
			      )
	.arg( channel_desc )              //1
	;

      QString loading_ratio  = QString::number( currAProf.lc_ratios[ i ] );
      QString ratio_tol      = QString::number( currAProf.lc_tolers[ i ] );
      QString volume         = QString::number( currAProf.l_volumes[ i ] );
      QString volume_tol     = QString::number( currAProf.lv_tolers[ i ] );
      QString data_end       = QString::number( currAProf.data_ends[ i ] );

      QString run_analysis;
      if ( currAProf.analysis_run[ i ] )
	run_analysis = tr("YES");
      else
	run_analysis = tr("NO");
      
      html_analysis_gen += tr(
				  "<table style=\"margin-left:30px\">"
				     "<tr><td> Loading Ratio:              </td>  <td> %1 </td> </tr>"
				     "<tr><td> Ratio Tolerance (&#177;%):  </td>  <td> %2 </td> </tr>"
				     "<tr><td> Loading Volume (&#181;l):   </td>  <td> %3 </td> </tr>"
				     "<tr><td> Volume Tolerance (&#177;%): </td>  <td> %4 </td> </tr>"
				     "<tr><td> Data End (cm):              </td>  <td> %5 </td> </tr>"
				  "</table>"
				  )
	.arg( loading_ratio )             //1
	.arg( ratio_tol )                 //2
	.arg( volume )                    //3
	.arg( volume_tol )                //4
	.arg( data_end )                  //5
	;

      
      html_analysis_gen    += tr(
				  "<table style=\"margin-left:30px\">"
				     "<tr><td> <i>Run Analysis:</i>        </td>  <td> %1 </td> </tr>"
				  "</table>"
				  )
	.arg( run_analysis )               //1
	;
				  
      bool triple_report = false;
      
      if ( currAProf.analysis_run[ i ] )
	{
	  //check what representative wvl is:
	  QString rep_wvl = QString::number( currAProf.wvl_edit[ i ] );
	  html_analysis_gen += tr(
				      "<table style=\"margin-left:50px\">"
				         "<tr><td> Wavelength for Edit, 2DSA-FM & Fitmen Stages (nm):   </td>  <td> %1 </td> </tr>"
				      "</table>"
				      )
	    .arg( rep_wvl )                //1
	    ;
	  
	  //now check if report will be run:
	  QString run_report;
	  if ( currAProf.report_run[ i ] )
	    {
	      run_report    = tr("YES");
	      triple_report = true;
	    }
	  else
	    run_report = tr("NO");

	  html_analysis_gen     += tr(
				      "<table style=\"margin-left:30px\">"
				         "<tr><td> <i> Run Report: </i>    </td>  <td> %1 </td> </tr>"
				      "</table>"
				      )
	    .arg( run_report)              //1
	    ;
	}

      if ( genMask_edited.ShowAnalysisGenParts[ "Channel General Settings" ].toInt()  )
	{
	  html_analysis_profile += html_analysis_gen;
	  html_analysis_gen.clear();
	}
      //End of the General Analysis Section

       
      //Separate Report | ReportItems table
      if ( triple_report )
	{
	  //QList < double > chann_wvls                = ch_wvls[ channel_desc ]; //ALEXEY: <-- BUG!
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc_alt ]; 
	  QMap < QString, US_ReportGMP > chann_reports = ch_reports[ channel_desc_alt ];
	    
	  int chann_wvl_number = chann_wvls.size();

	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl            = QString::number( chann_wvls[ jj ] );
	      QString triple_name    = channel_desc.split(":")[ 0 ] + "/" + wvl;
	      US_ReportGMP reportGMP = chann_reports[ wvl ];

	      //Exp. duration entered in the Channel Report Editor
	      QList< int > hms_tot;
	      double total_time = reportGMP.experiment_duration;
	      US_RunProtocol::timeToList( total_time, hms_tot );
	      QString exp_dur_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";

	      if ( genMask_edited.ShowAnalysisGenParts[ "Report Parameters (per-triple)" ].toInt()  )
		{
		  html_analysis_profile += tr(
					      "<table style=\"margin-left:50px\">"
					      "<tr><td> <b><i> Report Parameters for Triple: </i> &nbsp;&nbsp;&nbsp; %1 </b></td> </tr>"
					      "</table>"
					      )
		    .arg( triple_name )                                             //1
		    ;
		  
		  html_analysis_profile += tr(
					      "<table style=\"margin-left:70px\">"
					      "<tr><td> Total Concentration:           </td>  <td> %1 </td> </tr>"
					      "<tr><td> Total Concentration Tolerance: </td>  <td> %2 </td> </tr>"
					      "<tr><td> RMSD (upper limit):            </td>  <td> %3 </td> </tr>"
					      "<tr><td> Average Intensity:             </td>  <td> %4 </td> </tr>"
					      "<tr><td> Experiment Duration:           </td>  <td> %5 </td> </tr>"
					      "<tr><td> Experiment Duration Tolerance: </td>  <td> %6 </td> </tr>"
					      "</table>"
					      )
		    .arg( QString::number( reportGMP.tot_conc ) )                    //1
		    .arg( QString::number( reportGMP.tot_conc_tol ) )                //2
		    .arg( QString::number( reportGMP.rmsd_limit )  )                 //3
		    .arg( QString::number( reportGMP.av_intensity )  )               //4
		    .arg( exp_dur_str )                                              //5
		    .arg( QString::number( reportGMP.experiment_duration_tol ) )     //6
		    ;
		}

	      //Now go over ReportItems for the current triple:
	      int report_items_number = reportGMP.reportItems.size();

	      for ( int kk = 0; kk < report_items_number; ++kk )
		{
		  US_ReportGMP::ReportItem curr_item = reportGMP.reportItems[ kk ];

		  if ( genMask_edited.ShowAnalysisGenParts[ "Report Item Parameters (per-triple)" ].toInt()  )
		    {
		      html_analysis_profile += tr(
						  "<table style=\"margin-left:90px\">"
						  "<tr><td> <b><i> Report Item #%1: </i> &nbsp;&nbsp; Type | Method Parameters </b></td> </tr>"
						  "</table>"
						  )
			.arg( QString::number( kk + 1 ) )                                 //1
			;
		      
		      html_analysis_profile += tr(
						  "<table style=\"margin-left:110px\">"
						  "<tr><td> Type:                  </td>  <td> %1 </td> </tr>"
						  "<tr><td> Method:                </td>  <td> %2 </td> </tr>"
						  "<tr><td> Range Low:             </td>  <td> %3 </td> </tr>"
						  "<tr><td> Range High:            </td>  <td> %4 </td> </tr>"
						  "<tr><td> Integration Value:     </td>  <td> %5 </td> </tr>"
						  "<tr><td> Tolerance (%):         </td>  <td> %6 </td> </tr>"
						  "<tr><td> Fraction of Total (%): </td>  <td> %7 </td> </tr>"
						  "</table>"
						  )
			.arg( curr_item.type )                                      //1
			.arg( curr_item.method   )                                  //2
			.arg( QString::number( curr_item.range_low )  )             //3
			.arg( QString::number( curr_item.range_high )  )            //4
			.arg( QString::number( curr_item.integration_val )  )       //5
			.arg( QString::number( curr_item.tolerance )  )             //6
			.arg( QString::number( curr_item.total_percent )  )         //7
			;
		    }
		}
	    }
	}
      
      html_analysis_profile += tr(
				  "<br>"
				  )
	;
    }
  html_analysis_profile += tr( "<hr>" ) ;
  //ANALYSIS: General settings && Reports: end
    
    
  //ANALYSIS: 2DSA per-channel settings: begin
  html_analysis_profile_2dsa = tr(
				  "<h3 align=left> Analysis Profile: 2DSA Controls </h3>"
				  "&nbsp;&nbsp;<br>"
				  )
    ;

  //Job Flow Summary:
  QString html_analysis_profile_2dsa_flow;
  html_analysis_profile_2dsa_flow += tr(
				   "<table>"		   
				    "<tr>"
				       "<td><b>Job Flow Summary:</b></td>"
				    "</tr>"
				  "</table>"
			)
    ;
  html_analysis_profile_2dsa_flow += tr(
				   "<table style=\"margin-left:10px\">"
				      //2DSA
				      "<tr>"
				         "<td> \"2DSA  (TI Noise)\": </td>"
				         "<td> Run?                  </td>"    
                                         "<td> %1                    </td>"
				         "<td> &nbsp;&nbsp;          </td>"
				         "<td> &nbsp;&nbsp;          </td>"
                                      "</tr>"

				      //2DSA-FM
				      "<tr>"
                                         "<td> \"2DSA-FM  (TI+RI Noise)\": </td>"
				         "<td> Run?                        </td>"
				         "<td> %2                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
                                      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Meniscus Grid Points:       </td>"
				         "<td> %3                          </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Meniscus Fit Range (cm):    </td>"
				         "<td> %4                          </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Fit Type:                   </td>"
				         "<td> %5                          </td>"
				      "</tr>"				   

				      //FITMEN
				      "<tr>"
				         "<td> \"FITMEN\":                 </td>"
				         "<td> Run?                        </td>"
				         "<td> %6                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Auto-pick?                  </td>"
				         "<td> %7                          </td>"
				      "</tr>"

				      //2DSA-IT
				      "<tr>"
				         "<td> \"2DSA-IT  (TI+RI Noise)\": </td>"
				         "<td> Run?                        </td>"
				         "<td> %8                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
                                      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Refinement Iterations:      </td>"
				         "<td> %9                          </td>"
				      "</tr>"
				   
				      //2DSA-MC
				      "<tr>"
				         "<td> \"2DSA-MC\":                </td>"
				         "<td> Run?                        </td>"
				         "<td> %10                         </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> MonteCarlo Iterations:      </td>"
				         "<td> %11                         </td>"
				      "</tr>"
				   
				   "</table>"
				   "<br>"
				   )
    .arg( ( cAP2.job1run ? tr( "YES" ) : tr( "NO" ) ) )       //1

    .arg( ( cAP2.job2run ? tr( "YES" ) : tr( "NO" ) ) )       //2
    .arg( QString::number( cAP2.grpoints ) )                  //3
    .arg( QString::number( cAP2.fitrng ) )                    //4
    .arg( cAP2.fmb )                                          //5

    .arg( ( cAP2.job3run ? tr( "YES" ) : tr( "NO" ) ) )       //6
    .arg( ( cAP2.job3auto ? tr( "YES" ) : tr( "NO" ) ) )      //7

    .arg( ( cAP2.job4run ? tr( "YES" ) : tr( "NO" ) ) )       //8
    .arg( ( QString::number( cAP2.rfiters ) ) )               //9
    
    .arg( ( cAP2.job5run ? tr( "YES" ) : tr( "NO" ) ) )       //10
    .arg( ( QString::number( cAP2.mciters ) ) )               //11	  
    ;

  if ( genMask_edited.ShowAnalysis2DSAParts[ "Job Flow Summary" ].toInt()  )
    html_analysis_profile_2dsa +=  html_analysis_profile_2dsa_flow;
  

  //Per-Channel params:
  QString html_analysis_profile_2dsa_per_channel;
  html_analysis_profile_2dsa_per_channel += tr(
				   "<table>"		   
				     "<tr>"
				       "<td><b>Per-Channel Profile:</b></td>"
				     "</tr>"
				   "</table>"
				   "<br>"
				   )
    ;
  int nchna_2dsa   = cAP2.parms.size();
  for ( int i = 0; i < nchna_2dsa; i++ )
    {
      html_analysis_profile_2dsa_per_channel += tr(
				       "<table>"		   
				         "<tr>"
				            "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
				          "</tr>"
				       "</table>"
				       )
	.arg( cAP2.parms[ i ].channel )            //1
	;


      QString s_data = QString::number( cAP2.parms[ i ].s_min ) + ", " +
	               QString::number( cAP2.parms[ i ].s_max ) + ", " +
                       QString::number( cAP2.parms[ i ].s_grpts );

      QString ff0_data = QString::number( cAP2.parms[ i ].k_min ) + ", " +
	               QString::number( cAP2.parms[ i ].k_max ) + ", " +
                       QString::number( cAP2.parms[ i ].k_grpts );

      QString grid_rep =  QString::number( cAP2.parms[ i ].gridreps );

      QString custom_grid = cAP2.parms[ i ].cgrid_name;

      QString vary_vbar = cAP2.parms[ i ].varyvbar ? tr( "YES" ) : tr( "no" );

      QString const_ff0 = QString::number( cAP2.parms[ i ].ff0_const );

      html_analysis_profile_2dsa_per_channel += tr(
                                       "<table style=\"margin-left:30px\">"
                                          "<tr><td> s Min, Max, Grid Points:     </td>  <td> %1 </td> </tr>"
                                          "<tr><td> f/f0 Min, Max, Grid Points:  </td>  <td> %2 </td> </tr>"
                                          "<tr><td> Grid Repetitions:            </td>  <td> %3 </td> </tr>"
                                          "<tr><td> Custom Grid:                 </td>  <td> %4 </td> </tr>"
                                          "<tr><td> Varying Vbar:                </td>  <td> %5 </td> </tr>"
                                          "<tr><td> Constant f/f0:               </td>  <td> %6 </td> </tr>"
                                       "</table>"
                                       )
	.arg( s_data )                     //1                                                                                 
        .arg( ff0_data )                   //2                                                                                 
	.arg( grid_rep )                   //3                                                                                 
        .arg( custom_grid )                //4                                                                                 
	.arg( vary_vbar )                  //5                                                                                 
        .arg( const_ff0 )                  //6
	;
    }

  if ( genMask_edited.ShowAnalysis2DSAParts[ "Per-Channel Profiles" ].toInt()  )
    html_analysis_profile_2dsa +=  html_analysis_profile_2dsa_per_channel;
  
  html_analysis_profile_2dsa += tr( "<hr>" ) ;
  //ANALYSIS: 2DSA per-channel settings: end
  
  
  //ANALYSIS: PCSA per-channel settings: begin 
  html_analysis_profile_pcsa = tr(
				  "<h3 align=left> Analysis Profile: PCSA Controls </h3>"
				  "&nbsp;&nbsp;<br>"
				  )
    ;

  QString html_analysis_profile_pcsa_flow;
  html_analysis_profile_pcsa_flow += tr(
				   "<table style=\"margin-left:10px\">"
				     //PCSA
				     "<tr>"
				         "<td> \"PCSA\":    </td>"
				         "<td> Run?         </td>"    
                                         "<td> %1           </td>"
				      "</tr>"
				   "</table>"
				   )
    .arg( cAPp.job_run ? tr( "YES" ) : tr( "no" ) )
    ;

  if ( genMask_edited.ShowAnalysisPCSAParts[ "Job Flow Summary" ].toInt()  )
    html_analysis_profile_pcsa +=  html_analysis_profile_pcsa_flow;
  //End of PCSA Flow
  

  QString html_analysis_profile_pcsa_per_channel;
  if ( cAPp.job_run )
    {
      int nchna_pcsa   = cAPp.parms.size();
      for ( int i = 0; i < nchna_pcsa; i++ )
	{
	  html_analysis_profile_pcsa_per_channel += tr(
					   "<table>"		   
					     "<tr>"
					        "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
					     "</tr>"
					   "</table>"
					   )
	    .arg( cAPp.parms[ i ].channel )            //1
	    ;

	  QString x_data =  cAPp.parms[ i ].x_type + ", " +
	                    QString::number( cAPp.parms[ i ].x_min ) + ", " +
	                    QString::number( cAPp.parms[ i ].x_max );
	  QString y_data =  cAPp.parms[ i ].y_type + ", " +
	                    QString::number( cAPp.parms[ i ].y_min ) + ", " +
	                    QString::number( cAPp.parms[ i ].y_max );	  
	  QString z_data =  cAPp.parms[ i ].z_type + ", " +
	                    QString::number( cAPp.parms[ i ].z_value );
	                 	  
	  html_analysis_profile_pcsa_per_channel += tr(
					   "<table style=\"margin-left:30px\">"
					     "<tr><td> Curve Type:                </td>  <td> %1 </td> </tr>"
					     "<tr><td> X Axis Type, Min, Max:     </td>  <td> %2 </td> </tr>"
					     "<tr><td> Y Axis Type, Min, Max:     </td>  <td> %3 </td> </tr>"
					     "<tr><td> Z Axis Type, Value:        </td>  <td> %4 </td> </tr>"
					     "<tr><td> Variations Count:          </td>  <td> %5 </td> </tr>"
					     "<tr><td> Grid Fit Iterations:       </td>  <td> %6 </td> </tr>"
					     "<tr><td> Curve Resolution Points:   </td>  <td> %7 </td> </tr>"
					     "<tr><td> Noise Type:                </td>  <td> %8 </td> </tr>"
					     "<tr><td> Tikhonov Regularization:   </td>  <td> %9 </td> </tr>"
					     "<tr><td> Tikhonov Alpha:            </td>  <td> %10 </td> </tr>"
					     "<tr><td> MonteCarlo Iterations:     </td>  <td> %11 </td> </tr>"
					   "</table>"
					   )
	    .arg( cAPp.parms[ i ].curv_type )                     //1
	    .arg( x_data )                                        //2
	    .arg( y_data )                                        //3
	    .arg( z_data )                                        //4
	    .arg( QString::number( cAPp.parms[ i ].varcount ) )   //5
	    .arg( QString::number( cAPp.parms[ i ].grf_iters ) )  //6
	    .arg( QString::number( cAPp.parms[ i ].creso_pts ) )  //7
	    .arg( cAPp.parms[ i ].noise_type )                    //8
	    .arg( cAPp.parms[ i ].treg_type )                     //9
	    .arg( QString::number( cAPp.parms[ i ].tr_alpha ) )   //10
	    .arg( QString::number( cAPp.parms[ i ].mc_iters ) )   //11
	;
	}
    }

  if ( genMask_edited.ShowAnalysisPCSAParts[ "Per-Channel Profiles" ].toInt()  )
    html_analysis_profile_pcsa +=  html_analysis_profile_pcsa_per_channel;
  
  html_analysis_profile_pcsa += tr( "<hr>" ) ;
  //ANALYSIS: PCSA per-channel settings: end
  //APROFILE: end
  
  
  //Main assembly: reportMask based
  //QString html_assembled = QString("");
  html_assembled +=
    html_header
    + html_title
    + html_paragraph_open;

  assemble_parts( html_assembled );

  QString html_paragraph_close = tr( "</p>" );
  html_assembled += html_paragraph_close;
  
}


//write PDF Report
void US_ReporterGMP::write_pdf_report( void )
{
  QString html_paragraph_close = tr( "</p>" );

  QString html_footer = tr( 
    "<div align=right>"
       "<br>End of report: <i>\"%1\"</i>"
    "</div>"
			    )
    .arg( currProto. protoname )
    ;
  
  html_assembled += html_footer;
   
  QTextDocument document;
  document.setHtml( html_assembled );
  
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QPrinter::Letter);

  QString fileName  = currProto. protoname + "-run" + runID + ".pdf";
  filePath  = US_Settings::tmpDir() + "/" + fileName;
  printer.setOutputFileName( filePath );
  printer.setFullPage(true);
  printer.setPageMargins(QMarginsF(0, 0, 0, 0));
  
  document.print(&printer);
}

//save trees' selections into internal structures
void US_ReporterGMP::gui_to_parms( void )
{
  //tree-to-json: genTree && json-to-genMask structure
  QString editedMask_gen = tree_to_json ( topItem );
  parse_edited_gen_mask_json( editedMask_gen, genMask_edited );

  //tree-to-json: perChanTree
  QString editedMask_perChan = tree_to_json ( chanItem );
  parse_edited_perChan_mask_json( editedMask_perChan, perChanMask_edited );

  // //DEBUG
  // exit(1);
}

//Pasre reportMask JSON
void US_ReporterGMP::parse_edited_gen_mask_json( const QString maskJson, GenReportMaskStructure & MaskStr )
{
  QJsonDocument jsonDoc = QJsonDocument::fromJson( maskJson.toUtf8() );
  QJsonObject json = jsonDoc.object();

  int has_sol_items = 0;
  int has_analysis_items = 0;

  MaskStr.ShowReportParts      .clear();
  MaskStr.ShowSolutionParts    .clear();
  MaskStr.ShowAnalysisGenParts .clear();
  MaskStr.ShowAnalysis2DSAParts.clear();
  MaskStr.ShowAnalysisPCSAParts.clear();
  MaskStr.has_anagen_items  = 0;
  MaskStr.has_ana2dsa_items = 0;
  MaskStr.has_anapcsa_items = 0;
  
  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();

      if ( value.isString() )
	{
	  if ( value.toString().toInt() )
	      MaskStr.ShowReportParts[ key ] = true;
	  else
	    MaskStr.ShowReportParts[ key ] = false;
	}
      else if ( value.isArray() )
	MaskStr.ShowReportParts[ key ] = true;  //for now

      //treat Solutions && Analysis: nested JSON
      if ( key.contains("Solutions") || key.contains("Analysis") )
	{
	   QJsonArray json_array = value.toArray();
	   for (int i=0; i < json_array.size(); ++i )
	     {
	       foreach(const QString& array_key, json_array[i].toObject().keys())
		 {
		   if (  key.contains("Solutions") )
		     {

		       qDebug() << "Parse_editedJsonGen: Solution: array_key, value: "
				<<  array_key
				<<  json_array[i].toObject().value(array_key).toString();
			       
		       
		       MaskStr.ShowSolutionParts[ array_key ] = json_array[i].toObject().value(array_key).toString();
		       if ( MaskStr.ShowSolutionParts[ array_key ].toInt() )
			 ++has_sol_items;
		     }
		   if (  key.contains("Analysis") )
		     {
		       QJsonObject newObj = json_array[i].toObject().value(array_key).toObject();
		      
		       foreach ( const QString& n_key, newObj.keys() )
			 {
			   QString analysis_cathegory_item_value  = newObj.value( n_key ).toString();
			   
			   if ( analysis_cathegory_item_value.toInt() )
			     ++has_analysis_items;
			   
			   if ( array_key.contains("General") )
			     {
			       MaskStr.ShowAnalysisGenParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysisGenParts[ n_key ].toInt() )
				 ++MaskStr.has_anagen_items;

			       qDebug() << "Parse_editedJsonGen: Analysis:Gen: n_key, value: "
					<<  n_key
					<<  MaskStr.ShowAnalysisGenParts[ n_key ];
			       
			     }
			   
			   if ( array_key.contains("2DSA") )
			     {
			       MaskStr.ShowAnalysis2DSAParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysis2DSAParts[ n_key ].toInt() )
				 ++MaskStr.has_ana2dsa_items;

			       qDebug() << "Parse_editedJsonGen: Analysis:2DSA: n_key, value: "
					<<  n_key
					<<  MaskStr.ShowAnalysis2DSAParts[ n_key ];
			       
			     }
			   
			   if ( array_key.contains("PCSA") ) 
			     {
			       MaskStr.ShowAnalysisPCSAParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysisPCSAParts[ n_key ].toInt() )
				 ++MaskStr.has_anapcsa_items;

			       qDebug() << "Parse_editedJsonGen: Analysis:PCSA: n_key, value: "
					<<  n_key
					<<  MaskStr.ShowAnalysisPCSAParts[ n_key ];
			     }
			 }
		     }
		 }
	     }
	   
	   //Set if to show "Solutions" based on children items
	   if ( key.contains("Solutions") &&  !has_sol_items )
	     MaskStr.ShowReportParts[ key ] = false;

	   if ( key.contains("Analysis") &&  !has_analysis_items )
	     MaskStr.ShowReportParts[ key ] = false;
	}
    }
}

//Pasre reportMask JSON: perChan
void US_ReporterGMP::parse_edited_perChan_mask_json( const QString maskJson, PerChanReportMaskStructure & MaskStr )
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson( maskJson.toUtf8() );
    QJsonObject json = jsonDoc.object();
    
    MaskStr.ShowChannelParts      .clear();
    MaskStr.ShowTripleParts       .clear();
    MaskStr.has_triple_items      .clear();

    
    foreach(const QString& key, json.keys())
      {
	int has_channel_items = 0;
	
	QJsonValue value = json.value(key);
	qDebug() << "Key = " << key << ", Value = " << value;//.toString();

	MaskStr.ShowChannelParts[ key ] = false;  //for now
	
	QJsonArray json_array = value.toArray();
	for (int i=0; i < json_array.size(); ++i )
	  {
	    foreach(const QString& array_key, json_array[i].toObject().keys())
	      {
		QJsonObject newObj = json_array[i].toObject().value(array_key).toObject();

		foreach ( const QString& n_key, newObj.keys() )
		  {
		    QString triple_item_value  = newObj.value( n_key ).toString();
		    
		    if ( triple_item_value.toInt() )
		      ++has_channel_items;
		    
		    MaskStr.ShowTripleParts[ key ][ array_key ][ n_key ] = triple_item_value;
		    if ( MaskStr.ShowTripleParts[ key ][ array_key ][ n_key ].toInt() )
		      ++MaskStr.has_triple_items[ key ][ array_key ];
		    
		    qDebug() << "Parse_editedJsonTriples: Channel/Triple: " <<  key << "/" << array_key  << ": " <<  "n_key, value: "
			     <<  n_key
			     <<  triple_item_value;
		  }
	      }
	  }

	if ( !has_channel_items )
	  MaskStr.ShowChannelParts[ key ] = false;
      }
}

//transform 3-level tree to JSON
QString US_ReporterGMP::tree_to_json( QMap < QString, QTreeWidgetItem * > topLevItems )
{
  QString mask_edited;
  mask_edited += "{";
  
  QMap < QString, QTreeWidgetItem * >::iterator top;
  for ( top = topLevItems.begin(); top != topLevItems.end(); ++top )
    {
      qDebug() << "Top item " << top.key() << " is " <<  int(top.value()->checkState(0)) << "\n";

      mask_edited += "\"" + top.key().trimmed() + "\":";
      
      int children_lev1 = top.value()->childCount();
      if ( !children_lev1 )
	{
	  mask_edited += "\"" + QString::number( int(top.value()->checkState(0)) ) + "\",";
	}
      else
	{
	  mask_edited += "[";
	  for( int i = 0; i < top.value()->childCount(); ++i )
	    {
	      qDebug() << "\tThe child's " << top.value()->child(i)->text(1) << ", state is: " << top.value()->child(i)->checkState(0);

	      mask_edited += "{\"" + top.value()->child(i)->text(1).trimmed() + "\":";

	      int children_lev2 = top.value()->child(i)->childCount();
	      if ( !children_lev2 )
		{
		  mask_edited += "\"" + QString::number( int(top.value()->child(i)->checkState(0)) ) + "\"}";
		  if ( i != top.value()->childCount()-1 )
		    mask_edited += ",";
		}
	      else
		{
		  mask_edited += "{";
		  for( int ii = 0; ii < top.value()->child(i)->childCount(); ++ii )
		    {
		      qDebug() << "\t\tThe child's " << top.value()->child(i)->child(ii)->text(1) << ", state is: " << top.value()->child(i)->child(ii)->checkState(0);

		      mask_edited += "\"" + top.value()->child(i)->child(ii)->text(1).trimmed() + "\":";
		      int children_lev3 = top.value()->child(i)->child(ii)->childCount();
		      if ( !children_lev3 )
			{
			  mask_edited += "\"" + QString::number( int(top.value()->child(i)->child(ii)->checkState(0)) ) + "\"";
			  if ( ii != top.value()->child(i)->childCount()-1 )
			    mask_edited += ",";
			}
		      else
			{
			  //Here 3th level of nestedness may be considered if needed... and so on...
			}
		      
		    }
		  mask_edited += "}}";
		  if ( i != top.value()->childCount()-1 )
		    mask_edited += ",";
		}
	    }
	  mask_edited += "],";
	}
    }
  mask_edited.chop(1);
  mask_edited += "}";

  qDebug() << "Edited Mask: " << mask_edited;

  return mask_edited;
}

//Format times
void US_ReporterGMP::format_needed_params()
{
  //Duration
  QList< int > hms_dur;
  double duration = currProto. rpSpeed. ssteps[0].duration;
  US_RunProtocol::timeToList( duration, hms_dur );
  duration_str = QString::number( hms_dur[ 0 ] ) + "d " + QString::number( hms_dur[ 1 ] ) + "h " + QString::number( hms_dur[ 2 ] ) + "m ";
  
  //Delay Stage
  QList< int > hms_delay_stage;
  double delay_stage = currProto. rpSpeed. ssteps[0].delay_stage;
  US_RunProtocol::timeToList( delay_stage, hms_delay_stage );
  delay_stage_str = //QString::number( hms_delay_stage[ 0 ] ) + "d " +
    QString::number( hms_delay_stage[ 1 ] ) + "h " + QString::number( hms_delay_stage[ 2 ] ) + "m ";

  //Total Time
  QList< int > hms_tot;
  double total_time = currProto. rpSpeed. ssteps[0].total_time;
  US_RunProtocol::timeToList( total_time, hms_tot );
  total_time_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";

  //UV-vis delay
  QList< int > hms_delay_uvvis;
  double delay_uvvis = currProto. rpSpeed. ssteps[0].delay;
  qDebug() << "Delay UV-vis: " << delay_uvvis;
  US_RunProtocol::timeToList( delay_uvvis, hms_delay_uvvis );
  delay_uvvis_str = QString::number( hms_delay_uvvis[ 0 ] ) + "d " + QString::number( hms_delay_uvvis[ 1 ] ) + "h " + QString::number( hms_delay_uvvis[ 2 ] ) + "m ";
  qDebug() << "Delay UV-vis str: " << delay_uvvis_str;

  //UV-vis scanint
  QList< int > hms_scanint_uvvis;
  double scanint_uvvis = currProto. rpSpeed. ssteps[0].scanintv;
  qDebug() << "ScanInt UV-vis: " << scanint_uvvis;
  US_RunProtocol::timeToList( scanint_uvvis, hms_scanint_uvvis );
  scanint_uvvis_str = //QString::number( hms_scanint_uvvis[ 0 ] ) + "d " +
    QString::number( hms_scanint_uvvis[ 1 ] ) + "h " + QString::number( hms_scanint_uvvis[ 2 ] ) + "m " + QString::number( hms_scanint_uvvis[ 3 ] ) + "s ";
  qDebug() << "ScanInt UV-vis str: " << scanint_uvvis_str;

  //Interference delay
  QList< int > hms_delay_int;
  double delay_int = currProto. rpSpeed. ssteps[0].delay_int;
  qDebug() << "Delay Interference: " << delay_int;
  US_RunProtocol::timeToList( delay_int, hms_delay_int );
  delay_int_str = QString::number( hms_delay_int[ 0 ] ) + "d " + QString::number( hms_delay_int[ 1 ] ) + "h " + QString::number( hms_delay_int[ 2 ] ) + "m ";
  qDebug() << "Delay Interference str: " << delay_int_str;
  
  //Interference scanint
  QList< int > hms_scanint_int;
  double scanint_int = currProto. rpSpeed. ssteps[0].scanintv_int;
  qDebug() << "ScanInt Interference: " << scanint_int;
  US_RunProtocol::timeToList( scanint_int, hms_scanint_int );
  scanint_int_str = //QString::number( hms_scanint_uvvis[ 0 ] ) + "d " +
    QString::number( hms_scanint_int[ 1 ] ) + "h " + QString::number( hms_scanint_int[ 2 ] ) + "m " + QString::number( hms_scanint_int[ 3 ] ) + "s ";
  qDebug() << "ScanInt Interference str: " << scanint_int_str;
}

//get current date
void US_ReporterGMP::get_current_date()
{
  QDate dNow(QDate::currentDate());
  QString fmt = "MM/dd/yyyy";
  
  current_date = dNow.toString( fmt );
  qDebug() << "Current date -- " << current_date;
}



//assemble parts of the PDF based on mask
void US_ReporterGMP::assemble_parts( QString & html_str )
{
  QMap < QString, bool >::iterator top;
  for ( top = genMask_edited.ShowReportParts.begin(); top != genMask_edited.ShowReportParts.end(); ++top )
    {
      qDebug() << "QMap key, val -- " << top.key() << top.value();
      
      if ( top.key().contains("General") && top.value() )
    	html_str += html_general;
      if ( top.key().contains("Rotor") && top.value() )
    	html_str += html_lab_rotor;
      if ( top.key().contains("Operator") && top.value() )
    	html_str += html_operator;
      if ( top.key().contains("Speed") && top.value() )
    	html_str += html_speed;
      if ( top.key().contains("Cells") && top.value() )
    	html_str += html_cells;

      if ( top.key().contains("Solutions") && top.value() )
	html_str += html_solutions;
          
      if ( top.key().contains("Optical Systems") && top.value() )
    	html_str += html_optical;      
      if ( top.key().contains("Ranges") && top.value() )
    	html_str += html_ranges;
      if ( top.key().contains("Scan Counts") && top.value() )
    	html_str += html_scan_count;

      //Analysis
      if ( top.key().contains("Analysis Profile") && top.value() )
    	{
	  if ( genMask_edited.has_anagen_items ) 
	    html_str += html_analysis_profile;
	  if ( genMask_edited.has_ana2dsa_items ) 
	    html_str += html_analysis_profile_2dsa;
	  if ( genMask_edited.has_anapcsa_items ) 
	    html_str += html_analysis_profile_pcsa ;
    	}
    }
}


//Fetch Solution details && add to html_solutions
void US_ReporterGMP::add_solution_details( const QString sol_id, const QString sol_comment, QString& html_solutions )
{
  //get Solution info by ID:
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) +  db.lastError() );
      
      return;
    }

  US_Solution*   solution = new US_Solution;
  int solutionID = sol_id.toInt();

  int status = US_DB2::OK;
  status = solution->readFromDB  ( solutionID, &db );

  // Error reporting
  if ( status == US_DB2::NO_BUFFER )
    {
      QMessageBox::information( this,
				tr( "Attention" ),
				tr( "The buffer this solution refers to was not found.\n"
				    "Please restore and try again.\n" ) );
    }
  
  else if ( status == US_DB2::NO_ANALYTE )
    {
      QMessageBox::information( this,
				tr( "Attention" ),
				tr( "One of the analytes this solution refers to was not found.\n"
				    "Please restore and try again.\n" ) );
    }
  
  else if ( status != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Database Problem" ),
			    tr( "Database returned the following error: \n" ) +  db.lastError() );
    }
  //End of reading Solution:

  //add general solution details to html_solutions string:
  html_solutions += tr(
		       "<table style=\"margin-left:5px\">"
		       "<caption align=left> <b><i>Solution Information </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:20px\">"
		         "<tr><td> Solution Name: </td>               <td> %1 </td> </tr>"
		         "<tr><td> Solution Comment: </td>            <td> %2 </td> </tr>"
		         "<tr><td> Common VBar (20&#8451;):</td>      <td> %3 </td> </tr>"
		         "<tr><td> Storage Temperature (&#8451;):</td><td> %4 </td> </tr>" 
		       "</table>"
		       )
    .arg( solution->solutionDesc )                          //1
    .arg( sol_comment )                                     //2
    .arg( QString::number( solution->commonVbar20 ))        //3
    .arg( QString::number( solution->storageTemp ))         //4
    ;
  

  //Get analytes information
  if ( genMask_edited.ShowSolutionParts[ "Analyte Information" ].toInt()  )
    {
      html_solutions += tr(
			   "<table style=\"margin-left:20px\">"
			   "<caption align=left> <b><i>Analytes Information</i></b> </caption>"
			   "</table>"
			   )
	;
    }
  
  QString analyte_gen_info;
  QString analyte_detailed_info;
  int num_analytes = solution->analyteInfo.size();
  for (int i=0; i < num_analytes; ++i )
    {
      //clear analyte's strings:
      analyte_gen_info = QString("");
      analyte_detailed_info = QString("");

     
      US_Analyte analyte = solution->analyteInfo[ i ].analyte;
      QString a_name     = analyte.description;
      QString a_amount   = QString::number( solution->analyteInfo[ i ].amount );

      analyte_gen_info += tr(
			     "<tr>"
			       "<td> <i>Analyte #%1: </i></td> &nbsp;&nbsp;&nbsp;&nbsp; "
			       "<td> Name: </td>  <td> %2</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			       "<td> Molar Ratio:</td> <td>%3</td>"
			     "</tr>"
			     )
	.arg( QString::number( i + 1 ) )  //1
	.arg( a_name )                    //2
	.arg( a_amount )                  //3
	;

      QString a_mw     = QString::number( analyte.mw ) + " D";
      QString a_vbar20 = QString::number( analyte.vbar20 );
      
      int seqlen       = analyte.sequence.length();
      QString seqsmry  = analyte.sequence;
      int total       = 0;
      
      if ( seqlen == 0 )
	seqsmry         = tr( "(empty)" );
      else
	{
	  seqsmry         = seqsmry.toLower()
	    .remove( QRegExp( "[\\s0-9]" ) );
	  seqlen          = seqsmry.length();
	  if ( seqlen > 25 )
	    {
	      seqsmry        = QString( seqsmry ).left( 10 ) + " ... "
		+ QString( seqsmry ).mid( seqlen - 10 );
	    }
	  //seqsmry          += "\n  ";
	  seqsmry          += "<br>  ";
	  
	  for ( int ii = 0; ii < 26; ii++ )
	    {
	      QString letter  = QString( QChar( 'a' + ii ) );
	      int lcount      = analyte.sequence.count( letter );
	      total          += lcount;
	      if ( lcount > 0 )
		{
		  seqsmry     += QString().sprintf( "%d", lcount )
		    + " " + letter.toUpper() + ", ";
		  //seqsmry          += "\n  ";
		}
	    }
	  seqsmry     += QString().sprintf( "%d", total ) + " tot";
	}

      QString a_type   = tr( "Carbohydrate/Other" );
      a_type           = analyte.type == ( US_Analyte::PROTEIN ) ? tr( "Protein" ) : a_type;
      a_type           = analyte.type == ( US_Analyte::DNA ) ? tr( "DNA" ) : a_type;
      a_type           = analyte.type == ( US_Analyte::RNA ) ? tr( "RNA" ) : a_type;

      //absorbing residues
      US_Math2::Peptide p;
      US_Math2::calc_vbar( p, analyte.sequence, 20.0 );
      analyte.mw         = ( analyte.mw == 0.0 ) ? p.mw : analyte.mw;
      
      // Absorbing residues
      int cys = int(p.c);
      int hao = int(p.j);
      int orn = int(p.o);
      int trp = int(p.w);
      int tyr = int(p.y);
      int all_abs = 0;
      all_abs = cys + hao + orn + trp + tyr;
      qDebug() << "Tot AAs: " << all_abs;
      QString absorbing_residues = tr( "(empty)" );
      if (all_abs > 0)
	{
	  absorbing_residues = "";
	  if ( cys > 0)
	    absorbing_residues += QString().sprintf( "%d", cys ) + " " + "Cys"  + ", ";
	  if ( hao > 0)
	 absorbing_residues += QString().sprintf( "%d", hao ) + " " + "Hao"  + ", ";
	  if ( orn > 0)
	    absorbing_residues += QString().sprintf( "%d", orn ) + " " + "Orn"  + ", ";
	  if ( trp > 0)
	    absorbing_residues += QString().sprintf( "%d", trp ) + " " + "Trp"  + ", ";
	  if ( tyr > 0)
	    absorbing_residues += QString().sprintf( "%d", tyr ) + " " + "Tyr"  + ", ";

	  absorbing_residues += QString().sprintf( "%d", all_abs ) + " tot";
	}

      //remeber to exclude absorbing residues for non-protein species
      
      analyte_detailed_info += tr(
				  "<tr> <td> Type: </td> <td>%1</td>              </tr>"
				  "<tr> <td> Molecular Weight: </td>  <td> %2</td></tr>"
				  "<tr> <td> Vbar (20 &#8451;): </td>  <td> %3</td></tr>"
				  "<tr> <td> Sequence Length:  </td>  <td> %4</td></tr>"
				  "<tr> <td> Sequence Summary: </td>  <td> %5</td></tr>"
				  )
	.arg( a_type )                        //1
	.arg( a_mw )                          //2 
	.arg( a_vbar20 )                      //3
	.arg( QString::number( seqlen  ) )    //4
	.arg( seqsmry  )                      //5
	;

      //add absorbig residues if PROTEINS
      if ( analyte.type == 0 )
	{
	  analyte_detailed_info += tr(
				      "<tr> <td> AAs absorbing at 280 nm: </td> <td>%1</td> </tr>"
				      "<tr> <td> E280: </td>  <td> %2</td>                  </tr>"
				      "<tr> <td> Extinction count: </td>  <td> %3</td>      </tr>"
				      )
	    .arg( absorbing_residues )
	    .arg( QString::number( analyte.extinction[ 280.0 ] ) )
	    .arg( QString::number( analyte.extinction.keys().count() ) )
	    ;
	}

      //add info on the current analyte to html_solutions string
      if ( genMask_edited.ShowSolutionParts[ "Analyte Information" ].toInt()  )
	{
	  html_solutions += tr(
			       "<table style=\"margin-left:40px\">"
			         "%1"
			       "</table>"
			       "<table style=\"margin-left:60px\">"
			         "%2"
			       "</table>"  
			       )
	    .arg( analyte_gen_info )
	    .arg( analyte_detailed_info )
	    ;
	}
      
    }


  //general buffer information
  QString buffer_gen_info      = QString("");
  QString buffer_detailed_info = QString("");
  US_Buffer buffer = solution->buffer;

  buffer_gen_info += tr(
			"<tr>"
			  "<td> Buffer Name: </td> <td> %1 </td> "
			"</tr>"
			     )
    .arg( buffer.description )                //1
    ;

  buffer_detailed_info += tr(
			     "<tr><td> Density (20&#8451;, g/cm<sup>3</sup>):  </td>   <td>%1</td>  </tr>"
			     "<tr><td> Viscosity (20&#8451;, cP): </td>   <td>%2</td>               </tr>"
			     "<tr><td> pH:       </td>   <td>%3 </td>                              </tr>"
			     "<tr><td> Compressibility:</td>   <td>%4</td>                         </tr>"			     
			     )
    .arg( QString::number( buffer.density ) )                    //1
    .arg( QString::number( buffer.viscosity ) )                  //2
    .arg( QString::number( buffer.pH ) )                         //3
    .arg( QString::number( buffer.compressibility ) )            //4
    ;

  //buffer components (if any)
  QString buffer_components_info = QString("");
  for ( int i=0; i < buffer.component.size(); ++i )
    {
      QString component_desc =
	buffer.component[i].name
	+ " (" + QString::number( buffer.concentration[ i ] ) + " "
	+ buffer.component[i].unit
	+ ")";


      buffer_components_info += tr(
				   "<tr><td> Component Name:</td> &nbsp;&nbsp;  <td>%1</td>  </tr>"
				   )
	.arg( component_desc )
	;
    }
  

      


  //append html string: buffer general info
  if ( genMask_edited.ShowSolutionParts[ "Buffer Information" ].toInt()  )
    {
      html_solutions += tr(
			   "<table style=\"margin-left:30px\">"
			   "<caption align=left> <b><i>Buffer Information</i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:70px\">"
		           "%1"
			   "</table>"
			   "<table style=\"margin-left:100px\">"
		           "%2"
			   "</table>"
			   "<table style=\"margin-left:130px\">"
		           "%3"
			   "</table>"  
			   "<br>"
			   )
	.arg( buffer_gen_info )
	.arg( buffer_detailed_info )
	.arg( buffer_components_info )
	;
    }
}
