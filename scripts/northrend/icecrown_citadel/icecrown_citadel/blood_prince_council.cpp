/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: blood_prince_council
SD%Complete: 50%
SDComment: by /dev/rsa
SDCategory: Icecrown Citadel
EndScriptData */
// Need implement vortexes, mobs and power orb
#include "precompiled.h"
#include "def_spire.h"

enum BossSpells
{
        SPELL_BERSERK                           = 47008,

        //Darkfallen Orb
        SPELL_INVOCATION_OF_BLOOD               = 70952,

        //Valanar
        SPELL_KINETIC_BOMB                      = 72053,
        NPC_KINETIC_BOMB_TARGET                 = 38458,
        NPC_KINETIC_BOMB                        = 38454,
        SPELL_KINETIC_BOMB_EXPLODE              = 72052,
        SPELL_SHOCK_VORTEX                      = 72037,
        NPC_SHOCK_VORTEX                        = 38422,
        SPELL_SHOCK_VORTEX_AURA                 = 71945,
        SPELL_SHOCK_VORTEX_2                    = 72039,

        //Taldaram
        SPELL_GLITTERING_SPARKS                 = 71807,
        SPELL_CONJURE_FLAME_1                   = 71718,
        NPC_BALL_OF_FLAMES_1                    = 38332,
        SPELL_CONJURE_FLAME_2                   = 72040,
        NPC_BALL_OF_FLAMES_2                    = 38451,
        SPELL_FLAMES_AURA                       = 71709,
        SPELL_FLAMES                            = 71393,

        //Keleseth
        SPELL_SHADOW_LANCE                      = 71405,
        SPELL_SHADOW_LANCE_2                    = 71815,
        SPELL_SHADOW_RESONANCE                  = 71943,
        SPELL_SHADOW_RESONANCE_AURA             = 71822,
        NPC_DARK_NUCLEUS                        = 38369,

};

struct MANGOS_DLL_DECL boss_valanar_iccAI : public BSWScriptedAI
{
    boss_valanar_iccAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    Creature* pBrother1;
    Creature* pBrother2;
    bool intro;
    bool invocated;

    void Reset() 
    {
        if(!m_pInstance) return;
        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
        intro = false;
        resetTimers();
        invocated = false;
    }

    void MoveInLineOfSight(Unit* pWho) 
    {
        ScriptedAI::MoveInLineOfSight(pWho);
        if(!m_pInstance || intro) return;
        if (pWho->GetTypeId() != TYPEID_PLAYER) return;
        m_pInstance->SetData(TYPE_EVENT, 800);
        debug_log("EventMGR: creature %u send signal %u ",m_creature->GetEntry(),m_pInstance->GetData(TYPE_EVENT));
        intro = true;
    }

    void KilledUnit(Unit* pVictim)
    {
    switch (urand(0,1)) {
        case 0:
               DoScriptText(-1631302,m_creature,pVictim);
               break;
        case 1:
               DoScriptText(-1631303,m_creature,pVictim);
               break;
        }
    }

    void JustReachedHome()
    {
        if (!m_pInstance) return;
            m_pInstance->SetData(TYPE_BLOOD_COUNCIL, FAIL);
            m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance) return;
        DoScriptText(-1631304,m_creature,pKiller);
        if (pBrother1 && pBrother2 && !pBrother1->isAlive() && !pBrother2->isAlive()) 
           {
                m_pInstance->SetData(TYPE_BLOOD_COUNCIL, DONE);
                m_creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                pBrother1->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                pBrother2->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
           }
            else  m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance) return;
        pBrother1 = (Creature*)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_TALDARAM));
        pBrother2 = (Creature*)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_KELESETH));
        if (pBrother1 && !pBrother1->isAlive()) pBrother1->Respawn();
        if (pBrother2 && !pBrother2->isAlive()) pBrother2->Respawn();
        if (pBrother1) pBrother1->SetInCombatWithZone();
        if (pBrother2) pBrother2->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_BLOOD_COUNCIL, IN_PROGRESS);
        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_pInstance) return;
        if (!m_creature || !m_creature->isAlive())
            return;

        if(pDoneBy->GetGUID() == m_creature->GetGUID()) return;

        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetHealth() >= uiDamage ? m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH) - uiDamage : 0);

        uiDamage /=3;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance) return;
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealth() > m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH)/3 &&
                                      m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH) != 0)
                m_creature->SetHealth(m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH)/3);

    switch (m_pInstance->GetData(DATA_BLOOD_INVOCATION))
        {
          case NPC_VALANAR:
                if (!invocated)
                {
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_0);
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_1);
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_2);
                    DoScriptText(-1631307,m_creature);
                    invocated = true;
                }
                timedCast(SPELL_KINETIC_BOMB, uiDiff);
                timedCast(SPELL_SHOCK_VORTEX_2, uiDiff);
                if (timedQuery(SPELL_INVOCATION_OF_BLOOD, uiDiff))
                {
                invocated = false;
                switch (urand(0,1))
                    case 0:
                           m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_TALDARAM);
                           break;
                    case 1:
                           m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_KELESETH);
                           break;
                }
                break;

          case 0:
                if (timedQuery(SPELL_INVOCATION_OF_BLOOD, uiDiff))
                {
                    m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_VALANAR);
                    DoScriptText(-1631306,m_creature);
                };
          default:
                if (hasAura(SPELL_INVOCATION_OF_BLOOD, m_creature))
                {
                    doRemove(SPELL_INVOCATION_OF_BLOOD);
                }
                timedCast(SPELL_KINETIC_BOMB, uiDiff);
                timedCast(SPELL_SHOCK_VORTEX, uiDiff);
                break;
        }

        if (timedQuery(SPELL_BERSERK, uiDiff))
        {
             doCast(SPELL_BERSERK);
             DoScriptText(-1631305,m_creature);
         };

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_valanar_icc(Creature* pCreature)
{
    return new boss_valanar_iccAI(pCreature);
}

struct MANGOS_DLL_DECL boss_taldaram_iccAI : public BSWScriptedAI
{
    boss_taldaram_iccAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    Creature* pBrother1;
    Creature* pBrother2;
    bool invocated;


    void Reset() {
        if(!m_pInstance) return;
        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
        resetTimers();
        invocated = false;
    }

    void JustReachedHome()
    {
        if (!m_pInstance) return;
            m_pInstance->SetData(TYPE_BLOOD_COUNCIL, FAIL);
            m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance) return;
        if (pBrother1 && pBrother2 && !pBrother1->isAlive() && !pBrother2->isAlive()) 
           {
                m_pInstance->SetData(TYPE_BLOOD_COUNCIL, DONE);
                m_creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                pBrother1->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                pBrother2->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
           }
            else  m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (!m_pInstance) return;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance) return;
        pBrother1 = (Creature*)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_VALANAR));
        pBrother2 = (Creature*)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_KELESETH));
        if (pBrother1 && !pBrother1->isAlive()) pBrother1->Respawn();
        if (pBrother2 && !pBrother2->isAlive()) pBrother2->Respawn();
        if (pBrother1) pBrother1->SetInCombatWithZone();
        if (pBrother2) pBrother2->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_BLOOD_COUNCIL, IN_PROGRESS);
        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_pInstance) return;
        if (!m_creature || !m_creature->isAlive())
            return;

        if(pDoneBy->GetGUID() == m_creature->GetGUID()) return;

        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetHealth() >= uiDamage ? m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH) - uiDamage : 0);

        uiDamage /=3;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance) return;
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealth() > m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH)/3 &&
                                      m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH) != 0)
                m_creature->SetHealth(m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH)/3);

    switch (m_pInstance->GetData(DATA_BLOOD_INVOCATION))
        {
          case NPC_TALDARAM:
                if (!invocated)
                {
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_0);
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_1);
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_2);
                    DoScriptText(-1631307,m_creature);
                    invocated = true;
                }
                if (timedQuery(SPELL_INVOCATION_OF_BLOOD, uiDiff))
                {
                invocated = false;
                switch (urand(0,1))
                    case 0:
                           m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_VALANAR);
                           break;
                    case 1:
                           m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_KELESETH);
                           break;
                }
                timedCast(SPELL_GLITTERING_SPARKS, uiDiff);
                timedCast(SPELL_CONJURE_FLAME_2, uiDiff);
                break;

          default:
                if (hasAura(SPELL_INVOCATION_OF_BLOOD, m_creature))
                {
                    doRemove(SPELL_INVOCATION_OF_BLOOD);
                }
                timedCast(SPELL_GLITTERING_SPARKS, uiDiff);
                timedCast(SPELL_CONJURE_FLAME_1, uiDiff);
                break;
        }


        if (timedQuery(SPELL_BERSERK, uiDiff)){
                 doCast(SPELL_BERSERK);
                 DoScriptText(-1631305,m_creature);
                 };

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_taldaram_icc(Creature* pCreature)
{
    return new boss_taldaram_iccAI(pCreature);
}

struct MANGOS_DLL_DECL boss_keleseth_iccAI : public BSWScriptedAI
{
    boss_keleseth_iccAI(Creature* pCreature) : BSWScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    Creature* pBrother1;
    Creature* pBrother2;
    bool invocated;

    void Reset() {
        if(!m_pInstance) return;
        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
        resetTimers();
        invocated = false;
    }

    void JustReachedHome()
    {
        if (!m_pInstance) return;
            m_pInstance->SetData(TYPE_BLOOD_COUNCIL, FAIL);
            m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
    }

    void JustDied(Unit* pKiller)
    {
        if (!m_pInstance) return;
        if (pBrother1 && pBrother2 && !pBrother1->isAlive() && !pBrother2->isAlive()) 
           {
                m_pInstance->SetData(TYPE_BLOOD_COUNCIL, DONE);
                m_creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                pBrother1->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                pBrother2->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
           }
            else  m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (!m_pInstance) return;
    }

    void Aggro(Unit* pWho)
    {
        if (!m_pInstance) return;
        pBrother1 = (Creature*)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_TALDARAM));
        pBrother2 = (Creature*)Unit::GetUnit((*m_creature),m_pInstance->GetData64(NPC_VALANAR));
        if (pBrother1 && !pBrother1->isAlive()) pBrother1->Respawn();
        if (pBrother2 && !pBrother2->isAlive()) pBrother2->Respawn();
        if (pBrother1) pBrother1->SetInCombatWithZone();
        if (pBrother2) pBrother2->SetInCombatWithZone();

        m_pInstance->SetData(TYPE_BLOOD_COUNCIL, IN_PROGRESS);
        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetMaxHealth()*3);
        DoStartMovement(pWho, 30.0f);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (!m_pInstance) return;
        if (!m_creature || !m_creature->isAlive())
            return;

        if(pDoneBy->GetGUID() == m_creature->GetGUID()) return;

        m_pInstance->SetData(DATA_BLOOD_COUNCIL_HEALTH, m_creature->GetHealth() >= uiDamage ? m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH) - uiDamage : 0);

        uiDamage /=3;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance) return;
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealth() > m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH)/3 &&
                                      m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH) != 0)
                m_creature->SetHealth(m_pInstance->GetData(DATA_BLOOD_COUNCIL_HEALTH)/3);

    switch (m_pInstance->GetData(DATA_BLOOD_INVOCATION))
        {
          case NPC_KELESETH:
                if (!invocated)
                {
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_0);
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_1);
                    doAura(SPELL_INVOCATION_OF_BLOOD, m_creature, EFFECT_INDEX_2);
                    DoScriptText(-1631307,m_creature);
                    invocated = true;
                };

                if (timedQuery(SPELL_INVOCATION_OF_BLOOD, uiDiff))
                {
                invocated = false;
                switch (urand(0,1))
                    case 0:
                           m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_VALANAR);
                           break;
                    case 1:
                           m_pInstance->SetData(DATA_BLOOD_INVOCATION, NPC_TALDARAM);
                           break;
                }
                timedCast(SPELL_SHADOW_LANCE_2, uiDiff);
                timedCast(SPELL_SHADOW_RESONANCE, uiDiff);
                break;

          default:
                if (hasAura(SPELL_INVOCATION_OF_BLOOD, m_creature))
                {
                    doRemove(SPELL_INVOCATION_OF_BLOOD);
                }
                timedCast(SPELL_SHADOW_LANCE, uiDiff);
                timedCast(SPELL_SHADOW_RESONANCE, uiDiff);
                break;
        }


        if (timedQuery(SPELL_BERSERK, uiDiff)){
                 doCast(SPELL_BERSERK);
                 DoScriptText(-1631305,m_creature);
                 };

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_keleseth_icc(Creature* pCreature)
{
    return new boss_keleseth_iccAI(pCreature);
}

struct MANGOS_DLL_DECL mob_dark_nucleusAI : public ScriptedAI
{
    mob_dark_nucleusAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_lifetimer;

    void Reset()
    {
        m_creature->SetRespawnDelay(7*DAY);
        m_creature->SetInCombatWithZone();
        m_lifetimer = 45000;
        m_creature->AddSplineFlag(SPLINEFLAG_WALKMODE);
        SetCombatMovement(true);
        DoCast(m_creature, SPELL_SHADOW_RESONANCE_AURA);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_BLOOD_COUNCIL) != IN_PROGRESS) 
              m_creature->ForcedDespawn();

/*        if (!m_creature->HasAura(SPELL_SHADOW_RESONANCE_AURA))
              DoCast(m_creature, SPELL_SHADOW_RESONANCE_AURA);
*/
        if (m_lifetimer <= uiDiff)
            m_creature->ForcedDespawn();
        else m_lifetimer -= uiDiff;

    }
};

CreatureAI* GetAI_mob_dark_nucleus(Creature* pCreature)
{
     return new mob_dark_nucleusAI (pCreature);
};

struct MANGOS_DLL_DECL mob_ball_of_flamesAI : public ScriptedAI
{
    mob_ball_of_flamesAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 grow_timer;
    float fPosX, fPosY, fPosZ;
    float m_Size;
    float m_Size0;
    bool finita;

    void Reset()
    {
        m_creature->SetRespawnDelay(7*DAY);
        m_creature->SetInCombatWithZone();
        finita = false;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        SetCombatMovement(false);
        m_creature->GetPosition(fPosX, fPosY, fPosZ);
        m_creature->GetRandomPoint(fPosX, fPosY, fPosZ, urand(40, 60), fPosX, fPosY, fPosZ);
        m_creature->GetMotionMaster()->MovePoint(1, fPosX, fPosY, fPosZ);
        m_creature->AddSplineFlag(SPLINEFLAG_WALKMODE);

        m_Size0 = m_creature->GetObjectScale();
        m_Size = m_Size0;
        grow_timer = 500;

        if (m_creature->GetEntry() == NPC_BALL_OF_FLAMES_2)
            DoCast(m_creature, SPELL_FLAMES_AURA);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (!m_pInstance || type != POINT_MOTION_TYPE) return;
        if (id != 1)
            m_creature->GetMotionMaster()->MovePoint(1, fPosX, fPosY, fPosZ);
            else 
            {
                DoCast(m_creature, SPELL_FLAMES);
                finita = true;
            }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_BLOOD_COUNCIL) != IN_PROGRESS || finita)
              m_creature->ForcedDespawn();

        if (m_creature->GetEntry() == NPC_BALL_OF_FLAMES_2)
        {
            if (!m_creature->HasAura(SPELL_FLAMES_AURA))
                DoCast(m_creature, SPELL_FLAMES_AURA);

            if (grow_timer <= uiDiff)
            {
                m_Size = m_Size*1.01;
                m_creature->SetObjectScale(m_Size);
                grow_timer = 500;
            } else grow_timer -= uiDiff;
        } else return;

    }
};

CreatureAI* GetAI_mob_ball_of_flames(Creature* pCreature)
{
     return new mob_ball_of_flamesAI (pCreature);
};

struct MANGOS_DLL_DECL mob_kinetic_bombAI : public ScriptedAI
{
    mob_kinetic_bombAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_lifetimer;
    bool finita;

    void Reset()
    {
        m_creature->SetRespawnDelay(7*DAY);
        m_creature->SetInCombatWithZone();
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
        m_creature->AddSplineFlag(SPLINEFLAG_FLYING);
        SetCombatMovement(false);
        m_lifetimer = 45000;
    }

/*
Place shock bomb movement here
*/

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_BLOOD_COUNCIL) != IN_PROGRESS || finita)
              m_creature->ForcedDespawn();

        if (m_lifetimer <= uiDiff)
        {
            finita = true;
//            DoCast(m_creature, SPELL_KINETIC_BOMB_EXPLODE);
        }
        else m_lifetimer -= uiDiff;

    }
};

CreatureAI* GetAI_mob_kinetic_bomb(Creature* pCreature)
{
     return new mob_kinetic_bombAI (pCreature);
};

struct MANGOS_DLL_DECL mob_shock_vortexAI : public ScriptedAI
{
    mob_shock_vortexAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint32 m_lifetimer;
    bool finita;

    void Reset()
    {
        m_creature->SetRespawnDelay(7*DAY);
        m_creature->SetInCombatWithZone();
        m_lifetimer = 10000;
        m_creature->AddSplineFlag(SPLINEFLAG_WALKMODE);
        SetCombatMovement(false);
        DoCast(m_creature, SPELL_SHOCK_VORTEX_AURA);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_pInstance || m_pInstance->GetData(TYPE_BLOOD_COUNCIL) != IN_PROGRESS || finita)
              m_creature->ForcedDespawn();

        if (!m_creature->HasAura(SPELL_SHOCK_VORTEX_AURA))
              DoCast(m_creature, SPELL_SHOCK_VORTEX_AURA);

        if (m_lifetimer <= uiDiff)
            finita = true;
        else m_lifetimer -= uiDiff;

    }
};

CreatureAI* GetAI_mob_shock_vortex(Creature* pCreature)
{
     return new mob_shock_vortexAI (pCreature);
};

void AddSC_blood_prince_council()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_taldaram_icc";
    newscript->GetAI = &GetAI_boss_taldaram_icc;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_keleseth_icc";
    newscript->GetAI = &GetAI_boss_keleseth_icc;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_valanar_icc";
    newscript->GetAI = &GetAI_boss_valanar_icc;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dark_nucleus";
    newscript->GetAI = &GetAI_mob_dark_nucleus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ball_of_flames";
    newscript->GetAI = &GetAI_mob_ball_of_flames;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_kinetic_bomb";
    newscript->GetAI = &GetAI_mob_kinetic_bomb;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shock_vortex";
    newscript->GetAI = &GetAI_mob_shock_vortex;
    newscript->RegisterSelf();
/*
    newscript = new Script;
    newscript->Name = "mob_kinetic_bomb_target";
    newscript->GetAI = &GetAI_mob_kinetic_bomb_target;
    newscript->RegisterSelf();
*/
}
