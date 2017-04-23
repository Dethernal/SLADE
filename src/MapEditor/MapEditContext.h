
#ifndef __MAP_EDITOR_H__
#define __MAP_EDITOR_H__

#include "Archive/Archive.h"
#include "Edit/Edit3D.h"
#include "Edit/Input.h"
#include "Edit/LineDraw.h"
#include "Edit/ObjectEdit.h"
#include "General/UI.h"
#include "ItemSelection.h"
#include "MapEditor.h"
#include "Renderer/Overlays/InfoOverlay3d.h"
#include "Renderer/Overlays/LineInfoOverlay.h"
#include "Renderer/Overlays/SectorInfoOverlay.h"
#include "Renderer/Overlays/ThingInfoOverlay.h"
#include "Renderer/Overlays/VertexInfoOverlay.h"
#include "Renderer/Renderer.h"
#include "SLADEMap/SLADEMap.h"


namespace UI { enum class MouseCursor; }

class MapCanvas;
class UndoManager;
class UndoStep;
class MCOverlay;

class MapEditContext
{
public:
	struct EditorMessage
	{
		string	message;
		long	act_time;
	};

	MapEditContext();
	~MapEditContext();

	SLADEMap&				map() { return map_; }
	MapEditor::Mode			editMode() const { return edit_mode_; }
	MapEditor::SectorMode	sectorEditMode() const { return sector_mode_; }
	double					gridSize();
	ItemSelection&			selection() { return selection_; }
	MapEditor::Item			hilightItem() const { return selection_.hilight(); }
	vector<MapSector*>&		taggedSectors() { return tagged_sectors_; }
	vector<MapLine*>&		taggedLines() { return tagged_lines_; }
	vector<MapThing*>&		taggedThings() { return tagged_things_; }
	vector<MapLine*>&		taggingLines() { return tagging_lines_; }
	vector<MapThing*>&		taggingThings() { return tagging_things_; }
	vector<MapThing*>&		pathedThings() { return pathed_things_; }
	bool					gridSnap() const { return grid_snap_; }
	UndoManager*			undoManager() const { return undo_manager_; }
	Archive::mapdesc_t&		mapDesc() { return map_desc_; }
	MapCanvas*				canvas() const { return canvas_; }
	MapEditor::Renderer&	renderer() { return renderer_; }
	MapEditor::Input&		input() { return input_; }

	void	setEditMode(MapEditor::Mode mode);
	void 	setPrevEditMode() { setEditMode(edit_mode_prev_); }
	void	setSectorEditMode(MapEditor::SectorMode mode);
	void 	cycleSectorEditMode();
	void	setCanvas(MapCanvas* canvas) { this->canvas_ = canvas; }

	// General
	bool	update(long frametime);

	// Map loading
	bool	openMap(Archive::mapdesc_t map);
	void	clearMap();

	// Selection/hilight
	void	showItem(int index);
	void	updateTagged();
	void	selectionUpdated();

	// Grid
	void		incrementGrid();
	void		decrementGrid();
	double		snapToGrid(double position, bool force = true);
	fpoint2_t	relativeSnapToGrid(fpoint2_t origin, fpoint2_t mouse_pos);

	// Item moving
	const vector<MapEditor::Item>&	movingItems() const { return move_items_; }
	fpoint2_t						moveVector() { return move_vec_; }
	bool							beginMove(fpoint2_t mouse_pos);
	void							doMove(fpoint2_t mouse_pos);
	void							endMove(bool accept = true);

	// Editing
	void	copyProperties(MapObject* object = nullptr);
	void	pasteProperties();
	void	splitLine(double x, double y, double min_dist = 64);
	void	flipLines(bool sides = true);
	void	correctLineSectors();
	void	changeSectorHeight(int amount, bool floor = true, bool ceiling = true);
	void	changeSectorLight(bool up, bool fine);
	void 	changeSectorTexture();
	void	joinSectors(bool remove_lines);
	void	changeThingType();
	void	thingQuickAngle(fpoint2_t mouse_pos);
	void	mirror(bool x_axis);
	void 	editObjectProperties();

	// Tag edit
	int		beginTagEdit();
	void	tagSectorAt(double x, double y);
	void	endTagEdit(bool accept = true);

	// Object creation/deletion
	void	createObject(double x, double y);
	void	createVertex(double x, double y);
	void	createThing(double x, double y);
	void	createSector(double x, double y);
	void	deleteObject();

	// Editing handlers
	LineDraw&	lineDraw() { return line_draw_; }
	ObjectEdit&	objectEdit() { return object_edit_; }
	Edit3D&		edit3d() { return edit_3d_; }

	// Copy/paste
	void	copy();
	void	paste(fpoint2_t mouse_pos);

	// Editor messages
	unsigned	numEditorMessages() const { return editor_messages_.size(); }
	string		editorMessage(int index);
	long		editorMessageTime(int index);
	void		addEditorMessage(string message);

	// Feature help text
	const vector<string>&	featureHelpLines() const { return feature_help_lines_; }
	void					setFeatureHelp(const vector<string>& lines);

	// Undo/Redo
	void	beginUndoRecord(string name, bool mod = true, bool create = true, bool del = true);
	void	beginUndoRecordLocked(string name, bool mod = true, bool create = true, bool del = true);
	void	endUndoRecord(bool success = true);
	void	recordPropertyChangeUndoStep(MapObject* object);
	void	doUndo();
	void	doRedo();
	void	resetLastUndoLevel() { last_undo_level_ = ""; }

	// Overlays
	MCOverlay*	currentOverlay() const { return overlay_current_; }
	bool		overlayActive();
	void 		closeCurrentOverlay(bool cancel = false) const;
	void		openSectorTextureOverlay(vector<MapSector*>& sectors);
	void 		openQuickTextureOverlay();
	void 		openLineTextureOverlay();
	bool		infoOverlayActive() const { return info_showing_; }
	void		updateInfoOverlay();
	void		drawInfoOverlay(const point2_t& size, float alpha);

	// Player start swapping
	void	swapPlayerStart3d();
	void	swapPlayerStart2d(fpoint2_t pos);
	void	resetPlayerStart();

	// Misc
	string	modeString(bool plural = true) const;
	bool	handleKeyBind(string key, fpoint2_t position);
	void	updateDisplay();
	void	updateStatusText();
	void	updateThingLists();
	void	setCursor(UI::MouseCursor cursor) const;
	void	forceRefreshRenderer();

private:
	SLADEMap			map_;
	MapCanvas*			canvas_;
	Archive::mapdesc_t	map_desc_;
	long				next_frame_length_;

	// Undo/Redo stuff
	UndoManager*	undo_manager_;
	UndoStep*		us_create_delete_;

	// Editor state
	MapEditor::Mode			edit_mode_;
	MapEditor::Mode			edit_mode_prev_;
	ItemSelection			selection_;
	int						grid_size_;
	MapEditor::SectorMode	sector_mode_;
	bool					grid_snap_;
	int						current_tag_;

	// Undo/Redo
	bool	undo_modified_;
	bool	undo_created_;
	bool	undo_deleted_;
	string	last_undo_level_;

	// Tagged items
	vector<MapSector*>	tagged_sectors_;
	vector<MapLine*>	tagged_lines_;
	vector<MapThing*>	tagged_things_;

	// Tagging items
	vector<MapLine*>	tagging_lines_;
	vector<MapThing*>	tagging_things_;

	// Pathed things
	vector<MapThing*>	pathed_things_;

	// Moving
	fpoint2_t				move_origin_;
	fpoint2_t				move_vec_;
	vector<MapEditor::Item>	move_items_;
	MapEditor::Item			move_item_closest_;

	// Editing
	LineDraw	line_draw_;
	Edit3D		edit_3d_;
	ObjectEdit	object_edit_;

	// Object properties and copy/paste
	MapThing*	copy_thing_;
	MapSector*	copy_sector_;
	MapLine*	copy_line_;

	// Editor messages
	vector<EditorMessage>	editor_messages_;

	// Feature help text
	vector<string>	feature_help_lines_;

	// Player start swap
	fpoint2_t	player_start_pos_;
	int			player_start_dir_;

	// Renderer
	MapEditor::Renderer	renderer_;

	// Input
	MapEditor::Input	input_;

	// Full-Screen Overlay
	MCOverlay*  overlay_current_;

	// Info overlays
	bool				info_showing_;
	VertexInfoOverlay	info_vertex_;
	LineInfoOverlay		info_line_;
	SectorInfoOverlay	info_sector_;
	ThingInfoOverlay	info_thing_;
	InfoOverlay3D		info_3d_;
};

#endif//__MAP_EDITOR_H__
