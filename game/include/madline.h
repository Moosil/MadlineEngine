//
// Created by School on 2025/3/12.
//

#ifndef CELESTEPET_MADLINE_H
#define CELESTEPET_MADLINE_H

#include <data_type.h>
#include <string>

class Madline {
public:
//	static ParticleType P__DASH_A;
//	static ParticleType P__DASH_B;
//	static ParticleType P__CASSETTE_FLY;
//	static ParticleType P__SPLIT;
//	static ParticleType P__SUMMIT_LAND_A;
//	static ParticleType P__SUMMIT_LAND_B;
//	static ParticleType P__SUMMIT_LAND_C;
	static constexpr float MAX_FALL = 160;
	static constexpr float MAX_RUN = 90;
	static constexpr float RUN_ACCEL = 1000;
	static constexpr float WALL_SLIDE_START_MAX = 20;
	static constexpr float BOOST_TIME = .25;
	static constexpr float CLIMB_MAX_STAMINA = 110;
	static constexpr float CLIMB_TIRED_THRESHOLD = 20;
	static constexpr float WALK_SPEED = 64;
	enum States {
		NORMAL = 0,
		CLIMB, DASH, SWIM, BOOST, RED_DASH, HIT_SQUASH, LAUNCH, PICKUP, DREAM_DASH,
		SUMMIT_LAUNCH, DUMMY, INTRO_WALK, INTRO_JUMP, INTRO_RESPAWN, INTRO_WAKE_UP,
		BIRD_DASH_TUTORIAL, FROZEN, REFLECTION_FALL, STAR_FLY, TEMPLE_FALL,
		CASSETTE_FLY, ATTRACT
	};
	static constexpr std::string TALK_SFX = "player_talk";
	glm::vec2 speed;
	int facing;
//	PlayerSprite sprite;
//	PlayerHair hair;
//	StateMachine stateMachine;
	glm::vec2 cameraAnchor;
	bool cameraAnchorIgnoreX;
	bool cameraAnchorIgnoreY;
	glm::vec2 cameraAnchorLerp;
	bool forceCameraUpdate;
//	Leader leader;
//	VertexLight light;
	int dashes;
	float stamina = CLIMB_MAX_STAMINA;
	bool strawberriesBlocked;
	glm::vec2 previousPosition;
	bool dummyAutoAnimate = true;
	glm::vec2 forceStrongWindHair;
	glm::vec2 overrideDashDirection;
	bool flipInReflection = false;
	bool justRespawned;  // True if the player hasn't moved since respawning
	bool dead;
	int strawberryCollectIndex = 0;
	float strawberryCollectResetTimer = 0;
	bool autoJump;
	float autoJumpTimer;
	glm::vec<2, int> dashDir;
//	Booster currentBooster;
	static Colour normalHairColour;
	static Colour flyPowerHairColour;
	static Colour usedHairColour;
	static Colour flashHairColour;
	static Colour twoDashesHairColour;
	Colour overrideHairColour;
	enum introTypes { Transition, Respawn, WalkInRight, WalkInLeft, Jump, WakeUp, Fall, TempleMirrorVoid, None };
	introTypes introType;
private:
	static constexpr float GRAVITY = 900;
	static constexpr float HALF_GRAV_THRESHOLD = 40;
	static constexpr float FAST_MAX_FALL = 240;
	static constexpr float FAST_MAX_ACCEL = 300;
	static constexpr float RUN_REDUCE = 400;
	static constexpr float AIR_MULT = .65;
	static constexpr float HOLDING_MAX_RUN = 70;
	static constexpr float HOLD_MIN_TIME = .35;
	static constexpr float BOUNCE_AUTO_JUMP_TIME = .1;
	static constexpr float DUCK_FRICTION = 500;
	static constexpr int DUCK_CORRECT_CHECK = 4;
	static constexpr float DUCK_CORRECT_SLIDE = 50;
	static constexpr float DODGE_SLIDE_SPEED_MULT = 1.2;
	static constexpr float DUCK_SUPER_JUMP_X_MULT = 1.25;
	static constexpr float DUCK_SUPER_JUMP_Y_MULT = .5;
	static constexpr float JUMP_GRACE_TIME = 0.1;
	static constexpr float JUMP_SPEED = -105;
	static constexpr float JUMP_H_BOOST = 40;
	static constexpr float VAR_JUMP_TIME = .2;
	static constexpr float CEILING_VAR_JUMP_GRACE = .05;
	static constexpr int UPWARD_CORNER_CORRECTION = 4;
	static constexpr float WALL_SPEED_RETENTION_TIME = .06;
	static constexpr int WALL_JUMP_CHECK_DIST = 3;
	static constexpr float WALL_JUMP_FORCE_TIME = .16;
	static constexpr float WALL_JUMP_H_SPEED = MAX_RUN + JUMP_H_BOOST;
	static constexpr float WALL_SLIDE_TIME = 1.2;
	static constexpr float BOUNCE_VAR_JUMP_TIME = .2;
	static constexpr float BOUNCE_SPEED = -140;
	static constexpr float SUPER_BOUNCE_VAR_JUMP_TIME = .2;
	static constexpr float SUPER_BOUNCE_SPEED = -185;
	static constexpr float SUPER_JUMP_SPEED = JUMP_SPEED;
	static constexpr float SUPER_JUMP_H = 260;
	static constexpr float SUPER_WALL_JUMP_SPEED = -160;
	static constexpr float SUPER_WALL_JUMP_VAR_TIME = .25;
	static constexpr float SUPER_WALL_JUMP_FORCE_TIME = .2;
	static constexpr float SUPER_WALL_JUMP_H = MAX_RUN + JUMP_H_BOOST * 2;
	static constexpr float DASH_SPEED = 240;
	static constexpr float END_DASH_SPEED = 160;
	static constexpr float END_DASH_UP_MULT = .75;
	static constexpr float DASH_TIME = .15;
	static constexpr float DASH_COOLDOWN = .2;
	static constexpr float DASH_REFILL_COOLDOWN = .1;
	static constexpr int DASH_H_JUMP_THRU_NUDGE = 6;
	static constexpr int DASH_CORNER_CORRECTION = 4;
	static constexpr int DASH_V_FLOOR_SNAP_DIST = 3;
	static constexpr float DASH_ATTACK_TIME = .3;
	static constexpr float BOOST_MOVE_SPEED = 80;
	static constexpr float DUCK_WIND_MULT = 0;
	static constexpr int WIND_WALL_DISTANCE = 3;
	static constexpr float REBOUND_SPEED_X = 120;
	static constexpr float REBOUND_SPEED_Y = -120;
	static constexpr float REBOUND_VAR_JUMP_TIME = .15;
	static constexpr float REFLECT_BOUND_SPEED = 220;
	static constexpr float DREAM_DASH_SPEED = DASH_SPEED;
	static constexpr int DREAM_DASH_END_WIGGLE = 5;
	static constexpr float DREAM_DASH_MIN_TIME = .1;
	static constexpr float CLIMB_UP_COST = 100 / 2.2;
	static constexpr float CLIMB_STILL_COST = 100. / 10;
	static constexpr float CLIMB_JUMP_COST = 110. / 4;
	static constexpr int CLIMB_CHECK_DIST = 2;
	static constexpr int CLIMB_UP_CHECK_DIST = 2;
	static constexpr float CLIMB_NO_MOVE_TIME = .1;
	static constexpr float CLIMB_UP_SPEED = -45;
	static constexpr float CLIMB_DOWN_SPEED = 80;
	static constexpr float CLIMB_SLIP_SPEED = 30;
	static constexpr float CLIMB_ACCEL = 900;
	static constexpr float CLIMB_GRAB_Y_MULT = .2;
	static constexpr float CLIMB_HOP_Y = -120;
	static constexpr float CLIMB_HOP_X = 100;
	static constexpr float CLIMB_HOP_FORCE_TIME = .2;
	static constexpr float CLIMB_JUMP_BOOST_TIME = .2;
	static constexpr float CLIMB_HOP_NO_WIND_TIME = .3;
	static constexpr float LAUNCH_SPEED = 280;
	static constexpr float LAUNCH_CANCEL_THRESHOLD = 220;
	static constexpr float LIFT_Y_CAP = -130;
	static constexpr float LIFT_X_CAP = 250;
	static constexpr float JUMP_THRU_ASSIST_SPEED = -40;
	static constexpr float INFINITE_DASHES_TIME = 2;
	static constexpr float INFINITE_DASHES_FIRST_TIME = .5;
	static constexpr float FLY_POWER_FLASH_TIME = .5;
	static constexpr float THROW_RECOIL = 80;
	static const glm::vec2 CARRY_OFFSET_TARGET;
	static constexpr float CHASER_STATE_MAX_TIME = 4;
	
//	Level level;
//	Collision onCollideH;
//	Collision onCollideV;
	bool onGround;
	bool wasOnGround;
	int moveX;
	bool flash;
	bool wasDucking;
	float idleTimer;
//	static Chooser<std::string> idleColdOptions = new Chooser<std::string>().Add("idleA", 5f).Add("idleB", 3f).Add("idleC", 1f);
//	static Chooser<std::string> idleNoBackpackOptions = new Chooser<std::string>().Add("idleA", 1f).Add("idleB", 3f).Add("idleC", 3f);
//	static Chooser<std::string> idleWarmOptions = new Chooser<std::string>().Add("idleA", 5f).Add("idleB", 3f);
//	Hitbox hurtbox;
	float jumpGraceTimer;
	float varJumpSpeed;
	float varJumpTimer;
	int forceMoveX;
	float forceMoveXTimer;
	int hopWaitX;   // If you climb hop onto a moving solid, snap to beside it until you get above it
	float hopWaitXSpeed;
	glm::vec<2, int> lastAim;
	float dashCooldownTimer;
	float dashRefillCooldownTimer;
	float wallSlideTimer = WALL_SLIDE_TIME;
	int wallSlideDir;
	float climbNoMoveTimer;
	glm::vec2 carryOffset;
	glm::vec2 deadOffset;
	float introEase;
	float wallSpeedRetentionTimer; // If you hit a wall, start this timer. If coast is clear within this timer, retain h-speed
	float wallSpeedRetained;
	int wallBoostDir;
	float wallBoostTimer;   // If you climb jump and then do a sideways input within this timer, switch to wall jump
	float maxFall;
	float dashAttackTimer;
//	List<ChaserState> chaserStates;
	bool wasTired;
//	std::set<Trigger> triggersInside;
	float highestAirY;
	bool dashStartedOnGround;
	bool fastJump;
	int lastClimbMove;
	float noWindTimer;
	float dreamDashCanEndTimer;
//	Solid climbHopSolid;
	glm::vec2 climbHopSolidPosition;
//	SoundSource wallSlideSfx;
//	SoundSource swimSurfaceLoopSfx;
	float playFootstepOnLand;
	float minHoldTimer;
//	Booster lastBooster;
	bool calledDashEvents;
	int lastDashes;
//	Sprite sweatSprite;
	int startHairCount;
	bool launched;
	float launchedTimer;
	float dashTrailTimer;
//	List<ChaserStateSound> activeSounds = new List<ChaserStateSound>();
//	FMOD.Studio.EventInstance idleSfx;
//	Hitbox normalHitbox = Hitbox(8, 11, -4, -11);
//	Hitbox duckHitbox = Hitbox(8, 6, -4, -6);
//	Hitbox normalHurtbox = Hitbox(8, 9, -4, -11);
//	Hitbox duckHurtbox = Hitbox(8, 4, -4, -6);
//	Hitbox starFlyHitbox = Hitbox(8, 8, -4, -10);
//	Hitbox starFlyHurtbox = Hitbox(6, 6, -3, -9);
	glm::vec2 normalLightOffset = glm::vec2(0, -8);
	glm::vec<2, int> duckingLightOffset = glm::vec2(0, -3);
//	List<Entity> temp = new List<Entity>();
	float hairFlashTimer;
	glm::vec<2, int> windDirection;
	float windTimeout;
	float windHairTimer;
//	MirrorReflection reflection;
};

const glm::vec2 Madline::CARRY_OFFSET_TARGET = glm::vec2(0, -12);

#endif//CELESTEPET_MADLINE_H
