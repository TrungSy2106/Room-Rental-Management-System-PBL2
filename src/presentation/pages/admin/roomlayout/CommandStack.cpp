#include "CommandStack.h"

void CommandStack::push(std::unique_ptr<ICommand> cmd)
{
    cmd->redo();
    redoStack_.clear();
    if (cleanIndex_ > static_cast<int>(undoStack_.size()))
        cleanIndex_ = -1;
    undoStack_.push_back(std::move(cmd));
    notify();
}

void CommandStack::undo()
{
    if (undoStack_.empty()) return;
    undoStack_.back()->undo();
    redoStack_.push_back(std::move(undoStack_.back()));
    undoStack_.pop_back();
    notify();
}

void CommandStack::redo()
{
    if (redoStack_.empty()) return;
    redoStack_.back()->redo();
    undoStack_.push_back(std::move(redoStack_.back()));
    redoStack_.pop_back();
    notify();
}

void CommandStack::clear()
{
    undoStack_.clear();
    redoStack_.clear();
    cleanIndex_ = 0;
    notify();
}

void CommandStack::setClean()
{
    const int newCleanIndex = static_cast<int>(undoStack_.size());
    if (cleanIndex_ == newCleanIndex) return;
    cleanIndex_ = newCleanIndex;
    notify();
}

void CommandStack::notify()
{
    if (onChange_) onChange_();
}
