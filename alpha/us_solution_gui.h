//! \file us_solution_gui.h
#ifndef US_SOLUTION_GUI_H
#define US_SOLUTION_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "us_help.h"

#include "us_minimize.h"
#include "us_extinction_gui.h"
#include "us_extinctfitter_gui.h"

//! \Class US_SolutionMgrSelect
//!      This class provides a tabbed entry for solution selection
class US_SolutionMgrSelect: public US_Widgets
{
   Q_OBJECT

   public:

      //! brief Solution Selection Tab. To 
      //! instantiate the class a calling function must
      //! provide the ID of the investigator.

      //! \param invID          A pointer to the current investigator ID
      //! \param select_db_disk Indicates whether the default search
      //!    is on the local disk or in the DB
      //! \param tmp_solution    Pointer for selected solution
      US_SolutionMgrSelect( int*, int*, US_Solution* );

      US_Solution*   solution;
      US_Solution*   tmp_solution;
      US_Solution    orig_solution;

      int*          personID;
      int*          db_or_disk;
      bool          from_db;

   signals:
      //! Currently selected solution is accepted by User
      void solutionAccepted( void );
      void selectionCanceled( void );

   private:

      int           dbg_level;
      US_Help       showHelp;

      QLineEdit*    le_bufferInfo;
      QLineEdit*    le_solutionDesc;
      QLineEdit*    le_commonVbar20;
      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;
      QLineEdit*    le_storageTemp;
      QLineEdit*    le_guid;
      QLineEdit*    le_amount;
      QLineEdit*    le_buffeInfo;
      QLineEdit*    le_search;

      QListWidget*  lw_solutions;
      QListWidget*  lw_analytes;

      QTextEdit*    te_notes;

      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;
      QPushButton*  pb_delete;
      QPushButton*  pb_spectrum;
      QPushButton*  pb_help;

      // For list widget
      class SolutionInfo
      {
         public:
         int        solutionID;
         QString    GUID;
         QString    description;
         QString    filename;
         int        index;
      };

      QList< SolutionInfo >  info;
      int                         investigatorID;
      int                         experimentID;
      int                         channelID;
      bool                        signal;
      bool                        autosave;
      bool                        changed;

      QStringList   filenames;
      QStringList   descriptions;
      QStringList   GUIDs;
      QStringList   solutionIDs;

      QMap< QListWidgetItem*, int > solutionMap;
      QMap< QListWidgetItem*, int > analyteMap;


   private slots:

      void search          ( const QString& = QString() );
      //void selectSolution     ( QListWidgetItem* );
      void reset           ( void );
      // void db_error( const QString& );
  
      //void query           ( void ); 

      // void read_solution    ( void ); 
      //void read_db         ( void ); 
      //void connect_error   ( const QString& );
      // bool solution_path    ( QString& ); 

      /* void accept_solution  ( void ); */
      /* void spectrum        ( void ); */
      /* void delete          ( void ); */
      /* void reject          ( void ); */
      /* void accept          ( void ); */

       
      /* void delete_disk     ( void ); */
      /* void delete_db       ( void ); */
      /* bool solution_in_use  ( QString& ); */
      /* void info_solution    ( void ); */
      /* void select_solution  ( QListWidgetItem* ); */
      /* void select_solution  ( ); */
      /* void read_from_disk  ( QListWidgetItem* ); */
      /* void read_from_db    ( QListWidgetItem* ); */
      /* void read_from_db    ( const QString&   ); */


//      void show_component  ( const QString&, double );


  
      
      /* void set_solution_type( int  ); */
      /* QString solution_info ( US_Solution* ); */
      /* QString solution_smry ( US_Solution* ); */
      /* void sequence        (void); */

      void help( void ) { showHelp.show_help( "solution_select.html" ); };

   public slots:
     //void init_solution		( void );
};





class US_GUI_EXTERN US_SolutionGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      /*! \brief Generic constructor for the US_SolutionGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param expID   An integer value that indicates the ID of
                         the associated experiment
          \param chID    An integer value that indicates the ID of
                         the channel used
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
          \param dataIn  A reference to a structure that contains
                         the currently selected c/c/w dataset.
          \param auto_save A boolean value indicating whether the caller
                         wants an automatic save at Accept.
      */
      US_SolutionGui( int  = 1,
                      int  = 1,
                      bool = false,
                      int  = US_Disk_DB_Controls::Default,
                      const US_Solution& = US_Solution(),
                      bool = true );

      //! A null destructor. 
      ~US_SolutionGui() {};


   signals:
      //! A signal to indicate that the solution density and viscosity
      //!  has been updated and the screen is closing.
      //! \param density   - new density of the solution
      //! \param viscosity - new viscosity of the solution
      void valueChanged( double density, double viscosity );

      //! A signal to indicate that the solution data
      //!  has been updated and the screen is closing.
      //! \param solution   - the updated solution data.
      void valueChanged( US_Solution solution );

      //! A signal to indicate that the current disk/db selection has changed. 
      //! Return the ID of the solution in the current database.  A
      //! value of -1 indicates the data was manually input or was
      //! returned from the local disk.
      //! \param solutionID - A string value of the returned ID
      //void valueSolutionID( const QString solutionID );
      void valueSolutionID( const int solutionID );

      /*! \brief The signal that is emitted when the user chooses
	to accept the current choices. This information is
	passed back to the calling function.
	
	\param solution A reference to a structure that contains the
	solution, buffer and analyte choices for a 
	single c/c/w combination 
      */
      void updateSolutionGuiSelection( US_Solution solution );
      
      /*! \brief The signal that is emitted when the user chooses
	to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelSolutionGuiSelection( void );

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

      
 private:
      int                         investigatorID;
      int                         experimentID;
      int                         channelID;
      bool                        autosave;
      bool                        changed;     
      QStringList   IDs;
      QStringList   descriptions;
      QStringList   GUIDs;
      QStringList   filenames;


      bool          signal;
      QString       guid;
      double        temperature;

      int           disk_or_db;
      int           personID;
      int           dbg_level;
      bool          from_db;
      bool          solutionCurrent;
      bool          manualUpdate;
      bool          view_shared;
      bool          access;

      QTabWidget*             tabWidget;
      US_SolutionMgrSelect*   selectTab;
      //US_SolutionMgrNew*      newTab;
      //US_SolutionMgrEdit*     editTab;
      //US_SolutionMgrSettings* settingsTab;

      //!< The currently active solution Data. 
      US_Solution solution;
      US_Solution orig_solution; // saves original solution upon entry,
                             //   is returned if cancel was pressed

   private slots:
      void checkTab         ( int  );
      void update_disk_or_db( bool );
      void update_personID  ( int  );

//      void sel_investigator   ( void );
//      void source_changed     ( bool );
      void value_changed      ( const QString& );
//      void assign_investigator( int  );
//      void synch_components   ( void );
      void editAnaAccepted    ( void );
      void editAnaCanceled    ( void );
      void newAnaAccepted     ( void );
      void newAnaCanceled     ( void );
      void solutionAccepted    ( void );
      void solutionRejected    ( void );
};
           

#endif