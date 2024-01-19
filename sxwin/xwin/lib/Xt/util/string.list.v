! $Xorg: string.list,v 1.3 2000/08/17 19:46:22 cpqbld Exp $
! NOTE: To maintain binary compatibility, you can only APPEND items within
!	a given list!!!!
!
! See the makestrs(1) manual.
!

#prefix Xt
#feature XTSTRINGDEFINES
#externref extern
#externdef 
! note that the trailing space is required in the #externdef line.
#ctmpl util/StrDefs.ct.v

#file StringDefs.h
#table XtStrings
#htmpl util/StrDefs.ht.v

Naccelerators
NallowHoriz
NallowVert
NancestorSensitive
Nbackground
NbackgroundPixmap
Nbitmap
NborderColor
Nborder borderColor
NborderPixmap
NborderWidth
Ncallback
Nchildren
Ncolormap
Ndepth
NdestroyCallback
NeditType
Nfile
Nfont
NforceBars
Nforeground
Nfunction
Nheight
Nhighlight
NhSpace
Nindex
NinitialResourcesPersistent
NinnerHeight
NinnerWidth
NinnerWindow
NinsertPosition
NinternalHeight
NinternalWidth
NjumpProc
Njustify
NknobHeight
NknobIndent
NknobPixel
NknobWidth
Nlabel
Nlength
NlowerRight
NmappedWhenManaged
NmenuEntry
Nname
Nnotify
NnumChildren
Norientation
Nparameter
Npixmap
NpopupCallback
NpopdownCallback
Nresize
NreverseVideo
Nscreen
NscrollProc
NscrollDCursor
NscrollHCursor
NscrollLCursor
NscrollRCursor
NscrollUCursor
NscrollVCursor
Nselection
NselectionArray
Nsensitive
Nshown
Nspace
Nstring
NtextOptions
NtextSink
NtextSource
Nthickness
Nthumb
NthumbProc
Ntop
Ntranslations
NunrealizeCallback
Nupdate
NuseBottom
NuseRight
Nvalue
NvSpace
Nwidth
Nwindow
Nx
Ny
CAccelerators
CBackground
CBitmap
CBoolean
CBorderColor
CBorderWidth
CCallback
CColormap
CColor
CCursor
CDepth
CEditType
CEventBindings
CFile
CFont
CForeground
CFraction
CFunction
CHeight
CHSpace
CIndex
CInitialResourcesPersistent
CInsertPosition
CInterval
CJustify
CKnobIndent
CKnobPixel
CLabel
CLength
CMappedWhenManaged
CMargin
CMenuEntry
CNotify
COrientation
CParameter
CPixmap
CPosition
CReadOnly
CResize
CReverseVideo
CScreen
CScrollProc
CScrollDCursor
CScrollHCursor
CScrollLCursor
CScrollRCursor
CScrollUCursor
CScrollVCursor
CSelection
CSensitive
CSelectionArray
CSpace
CString
CTextOptions
CTextPosition
CTextSink
CTextSource
CThickness
CThumb
CTranslations
CValue
CVSpace
CWidth
CWindow
CX
CY
RAcceleratorTable
RAtom
RBitmap
RBool
RBoolean
RCallback
RCallProc
RCardinal
RColor
RColormap
RCursor
RDimension
RDisplay
REditMode
REnum
RFile
RFloat
RFont
RFontStruct
RFunction
RGeometry
RImmediate
RInitialState
RInt
RJustify
RLongBoolean Bool
RObject
ROrientation
RPixel
RPixmap
RPointer
RPosition
RScreen
RShort
RString
RStringArray
RStringTable
RUnsignedChar
RTranslationTable
RVisual
RWidget
RWidgetClass
RWidgetList
RWindow
Eoff
Efalse
Eno
Eon
Etrue
Eyes
Evertical
Ehorizontal
EtextRead read
EtextAppend append
EtextEdit edit
Extdefaultbackground
Extdefaultforeground
Extdefaultfont
NfontSet
RFontSet
CFontSet

#table XtStringsR6

RGravity
NcreateHook
NchangeHook
NconfigureHook
NgeometryHook
NdestroyHook
Nshells
NnumShells
RCommandArgArray
RDirectoryString
REnvironmentArray
RRestartStyle
RSmcConn
Hcreate
HsetValues
HmanageChildren
HunmanageChildren
HmanageSet
HunmanageSet
HrealizeWidget
HunrealizeWidget
HaddCallback
HaddCallbacks
HremoveCallback
HremoveCallbacks
HremoveAllCallbacks
HaugmentTranslations
HoverrideTranslations
HuninstallTranslations
HsetKeyboardFocus
HsetWMColormapWindows
HsetMappedWhenManaged
HmapWidget
HunmapWidget
Hpopup
HpopupSpringLoaded
Hpopdown
Hconfigure
HpreGeometry
HpostGeometry
Hdestroy

#file Shell.h
#table XtShellStrings
#htmpl util/Shell.ht.v

NiconName
CIconName
NiconPixmap
CIconPixmap
NiconWindow
CIconWindow
NiconMask
CIconMask
NwindowGroup
CWindowGroup
Nvisual
CVisual
NtitleEncoding
CTitleEncoding
NsaveUnder
CSaveUnder
Ntransient
CTransient
NoverrideRedirect
COverrideRedirect
NtransientFor
CTransientFor
NiconNameEncoding
CIconNameEncoding
NallowShellResize
CAllowShellResize
NcreatePopupChildProc
CCreatePopupChildProc
Ntitle
CTitle
RAtom
Nargc
CArgc
Nargv
CArgv
NiconX
CIconX
NiconY
CIconY
Ninput
CInput
Niconic
CIconic
NinitialState
CInitialState
Ngeometry
CGeometry
NbaseWidth
CBaseWidth
NbaseHeight
CBaseHeight
NwinGravity
CWinGravity
NminWidth
CMinWidth
NminHeight
CMinHeight
NmaxWidth
CMaxWidth
NmaxHeight
CMaxHeight
NwidthInc
CWidthInc
NheightInc
CHeightInc
NminAspectY
CMinAspectY
NmaxAspectY
CMaxAspectY
NminAspectX
CMinAspectX
NmaxAspectX
CMaxAspectX
NwmTimeout
CWmTimeout
NwaitForWm waitforwm
CWaitForWm Waitforwm

#table XtShellStringsR6

Nwaitforwm
CWaitforwm
NclientLeader
CClientLeader
NwindowRole
CWindowRole
Nurgency
CUrgency
NcancelCallback
NcloneCommand
CCloneCommand
Nconnection
CConnection
NcurrentDirectory
CCurrentDirectory
NdieCallback
NdiscardCommand
CDiscardCommand
Nenvironment
CEnvironment
NinteractCallback
NjoinSession
CJoinSession
NprogramPath
CProgramPath
NresignCommand
CResignCommand
NrestartCommand
CRestartCommand
NrestartStyle
CRestartStyle
NsaveCallback
NsaveCompleteCallback
NsessionID
CSessionID
NshutdownCommand
CShutdownCommand
NerrorCallback

