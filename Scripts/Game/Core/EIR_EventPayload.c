// Base payload class for EIR_EventBus events.
// Extend this to attach typed data to any fired event.
//
// Example:
//   class EIR_QuestCompletedPayload : EIR_EventPayload
//   {
//       string questId;
//       string playerGuid;
//   }
//
//   EIR_QuestCompletedPayload p = new EIR_QuestCompletedPayload();
//   p.questId = "sweep_stary";
//   p.playerGuid = "5";
//   EIR_EventBus.GetInstance().Fire("quest.completed", p);
//
// On the subscriber side, cast back to the concrete type:
//   void OnQuestCompleted(string eventName, EIR_EventPayload payload)
//   {
//       EIR_QuestCompletedPayload qp = EIR_QuestCompletedPayload.Cast(payload);
//       if (!qp)
//           return;
//       // use qp.questId, qp.playerGuid
//   }

class EIR_EventPayload
{
}
