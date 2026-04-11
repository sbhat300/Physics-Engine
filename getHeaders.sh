#!/bin/bash
rsync -avm --exclude='/EngineIncludes' --include='*/' --include='*.h' --include='*.hpp' --include='*.inl' --exclude='*' ./include/ ./EngineIncludes/