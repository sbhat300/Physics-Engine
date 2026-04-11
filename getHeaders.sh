#!/bin/bash
rsync -avm --exclude='/headers' --include='*/' --include='*.h' --exclude='*' ./ ./headers/