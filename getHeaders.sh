#!/bin/bash
rsync -avm --exclude='/headers' --include='*/' --include='*.h' --include='*.hpp' --include='*.inl' --exclude='*' ./ ./headers/