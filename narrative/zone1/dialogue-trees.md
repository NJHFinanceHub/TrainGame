# Zone 1: Dialogue Trees — Key NPCs

> Dialogue follows a hub-and-spoke model: each NPC has a base state with available topics. Topics unlock based on player progress, reputation, and choices. Critical dialogue uses a cinematic close-up camera.

---

## Elder Gilliam

**Role:** Leader of the Tail. Council head. Revolutionary figurehead. Secret collaborator turned genuine rebel.

**Voice:** Measured, warm, weary. Speaks in parables and careful phrasing. Never raises his voice — doesn't need to. Every word carries the weight of someone who has done terrible things and is trying to do one good thing before he dies.

**Physical:** Old. Missing one arm (amputation, self-inflicted during the early years — the "dark time" when Tailies ate their own). Walks with a cane. Eyes are sharp despite everything.

### First Meeting (Car 5, Council Meeting)

```
[GILLIAM sits at the head of the council table. The player approaches.]

GILLIAM: You're the one Grey says fights smart. Good. We don't need
         more martyrs. We need someone who can think while bleeding.

  → [Who are you?]
     GILLIAM: I'm the man who's kept this section alive for seventeen years.
              That's not a boast — it's a confession. Every day we survive
              here is a day I chose to wait instead of act. That changes now.

  → [What's the plan?]
     GILLIAM: The front is weakening. I have a source — no, I won't say who.
              What matters is: the Jackboots are stretched thin. They've
              been pulling soldiers forward. Something is happening up there
              that scares them more than us. We strike while they're distracted.

       → [How do you know this?]
          GILLIAM: (pause) I know the train better than anyone in this car.
                   Leave it at that. When this is over, I'll tell you
                   everything. You have my word.

       → [What do you need from me?]
          GILLIAM: A small team. Fast. Smart. You push forward, car by car.
                   The rest of us hold what you take. You're the spear —
                   we're the shield.

  → [Why me?]
     GILLIAM: Because you still have choices. Look at us — Martha counts
              rations, Samuel sharpens weapons, Asha patches wounds. We've
              each become our function. You're still undefined. That's power.

  → [I'm ready.]
     GILLIAM: No one is ready for this. But willing is enough.
              (stands, addresses the Council)
              It begins. May the Engine forgive us.
              (to the player, quietly)
              Stay after. I have something for you.
```

### Private Word (After Council Meeting)

```
[The Council disperses. GILLIAM gestures the player to his alcove.]

GILLIAM: (pulls a folded paper from his coat)
         Numbers. They won't mean anything to you yet.
         When you reach Car 14 — the Martyr's Gate — you'll find
         a monument. The numbers open something at its base.

  → [What's inside?]
     GILLIAM: The truth. About this place. About me.
              I'd tell you now, but you wouldn't believe it.
              You need to see the Gate first. You need to understand
              what we've done — what I've done — before the words
              will mean anything.

  → [Why trust me with this?]
     GILLIAM: Because you're going to the front. And at the front,
              you'll meet someone who will tell you a version of
              the truth designed to break you. I'd rather you hear
              my version first. It's uglier, but it's mine.

  → [What if I don't come back?]
     GILLIAM: (smiles, sadly)
              Then the numbers die with the next revolt.
              That might be for the best. I haven't decided.

  → [I'll find it.]
     GILLIAM: I know you will.
              (grips the player's shoulder with his remaining hand)
              Whatever you find... remember that people can change.
              Even old men who helped build prisons.
```

### Confrontation (Car 5, if player returns after finding the Confession)

```
[The player returns to Car 5 with the recording. GILLIAM is alone.]

GILLIAM: (sees the player's face)
         You found it.

  → [You designed this. All of it.]
     GILLIAM: (sits heavily) Yes.

       → [The class system was your idea.]
          GILLIAM: Wilford wanted to freeze the excess population.
                   Literally — put them outside. I said no. I said
                   give them a section, give them protein, let them live.
                   I thought I was being merciful.
                   (looks at his missing arm)
                   Mercy. What a word for a cage.

       → [The revolts are staged.]
          GILLIAM: Were. The first two, yes. Population control disguised
                   as hope. Wilford and I would agree: this many die,
                   this many survive, the pressure valve releases.
                   The third revolt — I lost control of it. People died
                   who weren't supposed to. That's when I understood
                   what I'd become.

       → [And this revolt?]
          GILLIAM: This one is real. I swear on every name on that
                   memorial wall. This one, I designed to succeed.
                   Against Wilford. Against the system I built.
                   (pause) Against myself, if it comes to that.

  → [CHOICE: Forgive]
     PLAYER: You've been carrying this a long time.
     GILLIAM: (surprised) You're not going to—
     PLAYER: You gave me the truth. You could have kept it buried.
             That's worth something.
     GILLIAM: (a long breath, the weight visibly lifts)
              Then let me give you everything. Everything I know about
              Wilford, the Engine, the train's systems, every secret
              passage and every weakness. I've been saving it for
              someone who could hear it and still move forward.
     [UNLOCKS: Gilliam's complete intelligence package — major gameplay benefit]

  → [CHOICE: Condemn]
     PLAYER: Seventy-four people died in that car because of you.
     GILLIAM: Yes.
     PLAYER: Children were taken. Families destroyed. And you sat at that
             table like a leader while knowing it was all theater.
     GILLIAM: Yes.
     PLAYER: You don't get to lead anymore.
     GILLIAM: (nods slowly) I know. I've known since before you were born.
              What will you tell them?
     [CHOICE CONTINUES: Tell the Council, Tell Everyone, or Just Remove Him Quietly]

  → [CHOICE: Keep the Secret]
     PLAYER: (long pause) ...No one else hears this.
     GILLIAM: You'd carry this? For me?
     PLAYER: Not for you. For them. The Tail needs you right now.
             After the revolution... we'll see.
     GILLIAM: (nods, understanding the conditional mercy)
              Then I will make sure this revolution is worth your silence.
     [GILLIAM becomes fully committed to the player's success — his guilt drives total loyalty]

  → [CHOICE: Destroy the Recording]
     PLAYER: (crushes the device)
     GILLIAM: Why?
     PLAYER: Some truths don't help anyone. You want to make it right?
             Get us to the Engine.
     GILLIAM: (watches the broken device on the floor)
              You're more like Wilford than you know.
     PLAYER: Don't ever say that again.
     GILLIAM: (quietly) No. I suppose not.
```

---

## Josie — Companion: The Protector

**Role:** Caretaker of the Nursery (Car 4). Fierce, compassionate, haunted by the children she couldn't save. If recruited, serves as the party's medic and moral compass.

**Voice:** Direct, warm with children, cold with enemies. Doesn't waste words. When she speaks about the missing children, her voice drops and hardens.

### Recruitment (Car 5, after Council Meeting)

```
[JOSIE approaches after the meeting, if the player helped with rations in Car 4.]

JOSIE: You fed my kids when no one asked you to.
       Now you're going forward. Toward the people who take them.

  → [I'll find out what happened to them.]
     JOSIE: (studies the player) I believe you.
            And I want to be there when you do.
            Not for revenge. For answers. And if the answer
            is what I think... then maybe revenge too.

  → [It's dangerous.]
     JOSIE: I've kept twelve children alive in a freezing metal box
            for six years. I've fought off Jackboots with my hands.
            Don't tell me about dangerous.

  → [I could use a medic.]
     JOSIE: I'm not a medic. I'm a mother who learned to stitch
            wounds because no one else would. But yes, I can
            keep you alive. Probably.

  → [Welcome aboard.]
     JOSIE: One condition. If we find the children — any of them —
            they come first. Before the revolution, before the Engine,
            before anything. Say yes or I stay here.

       → [Yes. Children first.]
          JOSIE: Then I'm yours. Until the Engine or the grave.
          [JOSIE joins as companion]

       → [I can't promise that.]
          JOSIE: (long pause, then nods)
                 Honest. I respect that more than a lie.
                 I'll come anyway. But you should know: if it comes
                 to a choice between your mission and those kids,
                 I won't ask permission.
          [JOSIE joins with CONDITIONAL LOYALTY flag]
```

### Combat Dialogue (Various)

```
[During fights, JOSIE calls out tactically:]

ON ENTERING COMBAT:
  "Stay tight. Watch your flanks in these corridors."
  "They bleed same as us. Remember that."

ON PLAYER TAKING DAMAGE:
  "Hold still — I've got you."
  "That's going to need stitches. Later."

ON PLAYER LOW HEALTH:
  "Get behind me. NOW."
  "Don't you dare die. I didn't come this far to lose another one."

ON KILLING AN ENEMY:
  (no celebration — clinical silence)
  "Check the body. They might have medicine."

ON COMPANION DOWN:
  "No no no — keep breathing, keep breathing—"
```

### Car 14 Reaction (Martyr's Gate)

```
[JOSIE examines the monument silently for a long time.]

JOSIE: My mother's name is on this wall.
       She was seventeen when the first revolt happened.
       (touches the wall)
       I was born six months after the massacre.
       In a car full of bodies they hadn't finished clearing.

  → [I'm sorry.]
     JOSIE: Don't be. She survived long enough to have me.
            That's more than most got.

  → [We'll make it mean something.]
     JOSIE: (looks at the player)
            You really believe that?

       → [I have to.]
          JOSIE: Good enough. I'll believe it when I see the Engine.

  → [Do you want to stop?]
     JOSIE: (shakes her head)
            This is exactly why I'm here.
            Every step forward is for her. For all of them.
```

---

## Pike — Companion: The Survivor

**Role:** Petty thief, scavenger, morally flexible. Saved from a beating in Car 2. Recruited as the party's scout and lockpick.

**Voice:** Fast-talking, nervous energy, dark humor. Covers fear with jokes. Loyal to whoever saved his skin but always calculating the odds.

### Recruitment (Car 5, after Council Meeting — only if saved in Car 2)

```
[PIKE sidles up to the player after the meeting.]

PIKE: Hey. So. The revolution. Fun.
      (fidgets)
      Look, I'm not a fighter. I'm not a leader.
      I'm the guy who knows where things are and how
      to get places he shouldn't be.

  → [I could use someone like that.]
     PIKE: Yeah? Because Grey says I'm a liability and
           Samuel calls me a rat. But you — you stopped them
           from beating me. So I figure I owe you.
           And Pike always pays his debts. Usually.

  → [Can I trust you?]
     PIKE: (genuinely considers this)
           With your life? Probably. I mean, if you die, I die.
           There's no exit here — it's a train.
           With your rations? I mean... I'll try.

  → [What can you do?]
     PIKE: I can open locks — any lock on this train that isn't
           electronic. I can fit through spaces nobody else can.
           I can talk my way past people who should shoot me.
           And I know every crawlspace, vent, and gap in the Tail.
           You want to go forward? I know routes nobody else does.

  → [You're in.]
     PIKE: (visible relief)
           Great. I mean, great for the revolution and stuff.
           Not just because the alternative was staying here
           and waiting for someone else to beat me up. That too.
     [PIKE joins as companion]
```

### Exploration Dialogue (Various)

```
[PIKE comments frequently during exploration:]

ON FINDING LOOT:
  "Ooh. That's worth something. Dibs. I mean — for the cause."
  "You know, in another life, I'd have been a professional shopper."

ON ENTERING A NEW CAR:
  "So, on a scale of one to the Dark Car, how scared should I be?"
  "I'll go first. Wait, no. You go first. I'll watch your back. Way back."

ON ENCOUNTERING JACKBOOTS:
  "Remember when my biggest problem was stealing protein blocks?"
  "There's a vent up there. Just saying."

ON MORAL CHOICES:
  "I'm staying out of this one. Last time I had an opinion, I got kicked."
  "Whatever you decide, I'll say it was my idea if it works."
```

---

## Brakeman — Companion: The Wall

**Role:** The Tail's strongest fighter. Simple, loyal, reliable. If the player showed mercy in Car 3, he follows out of gratitude and respect.

**Voice:** Few words. Speaks in short, declarative sentences. Not unintelligent — he's just economical with language.

### Recruitment (Car 5, after Council Meeting — only if shown mercy in Car 3)

```
[BRAKEMAN stands by the door, waiting. He's huge.]

BRAKEMAN: You pulled that punch. In the ring.

  → [You noticed.]
     BRAKEMAN: I'm slow. Not stupid.
               You could have broken my arm. Used the grate.
               Grey even nodded at you to do it.
               You didn't.

  → [You're a good fighter. I didn't want to break you.]
     BRAKEMAN: (nods) Respect.
               I want to come with you.

  → [Can you fight? Really fight?]
     BRAKEMAN: I stopped eight Jackboots during the third revolt.
               Four of them didn't get up.
               (shows scarred knuckles)
               I can fight.

  → [Welcome.]
     BRAKEMAN: (simply) I'll be your wall. You go forward.
               Nothing gets past me.
     [BRAKEMAN joins as companion]
```

### Combat Dialogue

```
ON ENTERING COMBAT:
  "Behind me."
  "I'll hold."

ON TAKING DAMAGE:
  (grunts — no complaints)
  "Felt that."

ON PROTECTING PLAYER:
  "NOT. HIM." (throws an enemy)
  "Stay behind the wall."

ON VICTORY:
  "Done."
  (cracks knuckles, says nothing)
```

---

## Commander Nix (Car 9 Boss)

**Voice:** Clipped, professional, contemptuous. She doesn't hate the Tailies — she doesn't think about them enough to hate them. They're a management problem.

### Pre-Battle (If Player Approaches Diplomatically)

```
NIX: (over loudspeaker)
     This is Commander Nix. You're in violation of Section 7
     of the Eternal Engine Compact. Return to your designated
     zone immediately.

  → [The Compact is a leash, not a law.]
     NIX: A leash assumes you're an animal.
          I was going to say "regulation." But sure.
          Come ahead. We'll regulate.

  → [People are starving back there.]
     NIX: People are starving everywhere.
          The difference is, back there you starve slowly.
          Come through this door and you starve fast.

  → [We don't want to fight.]
     NIX: (genuine surprise, then cold laugh)
          Yes you do. You've been building weapons for months.
          We watched. It was almost charming.

  → [Stand down and no one gets hurt.]
     NIX: Stand down? This is my post. This is my career.
          If I stand down, I go to the Tail.
          (loads taser)
          I'd rather shoot you.
```

### Post-Battle (If Captured)

```
[NIX is restrained, bloodied but defiant.]

NIX: (spits blood)
     So. The rats got out.
     Congratulations. You've taken four cars of a
     thousand-car train.

  → [Tell me about the forward defenses.]
     NIX: Or what? You'll kill me? Kill me then.
          I was dead the moment that door opened.

       → [We won't kill you. We're not like you.]
          NIX: (studies the player, something shifts)
               No. Maybe you're not.
               ...Third Class has forty soldiers. Spread thin.
               The real defense is at Second Class. That's where
               the guns are. Real guns. Not this taser garbage.

  → [Who ordered the children taken?]
     NIX: (goes quiet — this is the one question that hits her)
          ...That comes from the top. We just execute the orders.
          I've never asked where they go.

       → [You never asked.]
          NIX: (flat) I never asked.
               (long pause)
               ...Engine car. That's the rumor. I don't know more.

  → [Why do you serve them?]
     NIX: Because the alternative is being you.
          (pauses) That's not bravado. I mean it.
          I've seen what happens to people with no purpose.
          The uniform gives me purpose. Without it, I'm just
          another body on a train going nowhere.
```

---

## Commander Volkov (Car 15 Boss)

**Voice:** Russian-accented, booming, theatrical. Unlike Nix's professionalism, Volkov is a true believer — he thinks the class system is divine order and the Tailies are an affront to the Engine's design.

### Pre-Battle

```
VOLKOV: (standing on the command platform, shock lance crackling)
        Seventeen years I've stood at this door.
        Every revolt ends here. Every one.
        You think you're different?

  → [Yes.]
     VOLKOV: (laughs) They all say that.
             Curtis said it. Maria said it. The ones whose
             names you scratched into that wall back there.
             They all thought they were the one.

  → [You're afraid.]
     VOLKOV: (stops laughing)
             I have stood at this door through three revolts.
             I've killed more of your people than you've met.
             Fear is not something I—
             (the lights flicker — a Kronole grenade? Forgotten allies? Revolt charge?)
             What was—

  → [Stand aside, Volkov. It's over.]
     VOLKOV: Over? Over? This is the ETERNAL ENGINE.
             Nothing is OVER. The train turns and turns and
             the wheel TURNS and the Engine TURNS and—
             (realizes he's ranting, composes himself)
             You'll die here. Like the rest.

  → [Attack without speaking.]
     [Combat begins immediately — no dialogue advantage but no enemy preparation either.]
```

### Defeat Dialogue

```
[VOLKOV is beaten, on his knees, shock lance broken.]

VOLKOV: (breathing hard, blood on his face)
        You... you don't understand.
        If you go through that door...
        if the Tail reaches Third Class...
        the whole system... everything that keeps
        this train alive...

  → [CHOICE: Execute]
     PLAYER: The system is already dead.
     (PLAYER ends it. Quick. The Tail cheers.)
     [VOLKOV dies. Forward sections are alerted by the silence on his radio.]

  → [CHOICE: Interrogate]
     PLAYER: Tell me everything about the forward defenses.
     VOLKOV: (laughs weakly) You want me to betray my post?
             My entire life has been this door.
     PLAYER: Your life is in my hands. Your door is open.
             Make a new life.
     VOLKOV: (long silence)
             ...Third Class is a powder keg. The workers hate
             the system as much as you do. But they fear
             falling to the Tail more. Give them a reason
             to join you and you won't need to fight your
             way through. You'll be invited.
     [UNLOCKS: Third Class political intelligence]

  → [CHOICE: Turn Him]
     PLAYER: You hate this post. Your own logs say it.
             You requested transfer six times. Help us
             and you'll never stand at another door again.
     VOLKOV: (shocked the player read his logs)
             ...You'd trust me?
     PLAYER: No. But I'd use you. Same as they did.
             At least I'm honest about it.
     VOLKOV: (grim smile)
             ...Fine. I'll radio ahead. Tell them the revolt
             failed. Buy you time. But the moment they check —
             and they will check — I'm dead either way.
     [VOLKOV becomes a temporary double agent — risky but high reward]
```

---

## Supporting NPC Dialogue Samples

### Old Maggie (Car 2)

```
MAGGIE: Seventeen years I've been grinding bugs into blocks.
        You know what the secret ingredient is?
        (leans in)
        Pretending it's not bugs.

  → [What's really in the blocks?]
     MAGGIE: Cockroach. Larvae. Some kind of binding agent
             that comes in a sealed drum — I don't know what
             it is and I don't ask. And lately...
             (glances around)
             ...something that makes people sleepy.
```

### Blind Yuri (Car 8)

```
YURI: Press your ear here. What do you hear?

  → [Metal. Vibration. The train.]
     YURI: You hear noise. I hear a heartbeat.
           Every car has its own rhythm. The Tail beats slow
           and angry. Third Class beats like a factory.
           First Class... First Class barely beats at all.
           Too much insulation. Too much comfort.
           But the Engine...
           (his face changes)
           The Engine beats like something alive.
```

### Zhora (Car 12)

```
ZHORA: You want to know why I do this?
       Because Asha has three doses of medicine
       and sixty sick people. I have enough Kronole
       to make them all forget they're dying.
       That's not dealing. That's hospice.

  → [It's still a drug den.]
     ZHORA: And the Protein Kitchen is still a bug grinder.
            And the Workshop is still an illegal weapons factory.
            We're all criminals on this train.
            At least my crimes come with pain relief.
```

### Anvil (Car 7)

```
ANVIL: (holds up a freshly forged weapon)
       This is a pipe wrapped in wire with a nail through it.
       In the old world, it would be garbage.
       In here, it's hope.
       (sets it on the rack with reverence)
       Every weapon I make is a promise that we won't
       go quietly. Not this time.
```

---

## Dialogue System Notes

**Reputation Tracking:** Every dialogue choice affects hidden reputation counters with factions and individuals. Major NPCs have trust thresholds that unlock deeper dialogue branches.

**Tone System:** The player's dialogue choices over time establish a "tone" that NPCs react to:
- **Pragmatic** — NPCs give practical advice, skip emotional appeals
- **Idealistic** — NPCs share hopes and fears, offer loyalty-based quests
- **Ruthless** — NPCs fear the player, offer information quickly but trust slowly
- **Compassionate** — NPCs open up emotionally, side quests become more personal

**Companion Interjections:** When companions are present, they react to dialogue choices with approval/disapproval barks and occasionally interrupt with their own questions or comments. This makes conversations feel alive and reminds the player their choices are observed.

**Silence Option:** Every dialogue tree has an implicit "[Stay silent]" option. NPCs react to silence based on their personality — some fill the gap, others wait, others get nervous. Silence is never a non-choice; it always communicates something.
