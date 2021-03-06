% Testing environment for a Constant Jerk Trajectory that covers a given
% distance with a given average velocity, starting at a given initial
% velocity.
% This trajectory is used in the R2E7 motor modules for smoothly moving
% through a series of given positions with specific average speeds and
% therefore taking an easily predictable amount of time.
function CJT_Sandbox()
    %% Trajectory Parameters:
    dir = 1;
    tr = 0;
    tc = 0;
    t3 = 0;
    ap = 0;
    Dv = 0;
    vp = 0;
    tcrit = [0 0 0];
    scrit = [0 0 0];

    %% Precompute
    function computeParameters(s, va, v0, vf)
        if nargin > 3 % Final velocity specified. Time constraint must be removed
            
        end
    end

end